#include <kore3/gpu/device.h>
#include <kore3/gpu/pipeline.h>
#include <kore3/gpu/sampler.h>
#include <kore3/image.h>
#include <kore3/io/filereader.h>
#include <kore3/math/matrix.h>
#include <kore3/system.h>

#include <kong.h>

#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

#define TEX_SIZE 256
#define MIP_LEVELS 4
#define FACE_COUNT 6

static const int width  = 1920;
static const int height = 1080;

static kore_gpu_device       device;
static kore_gpu_command_list list;
static vertex_in_buffer      vertices;
static kore_gpu_buffer       indices;
static kore_gpu_texture      test_texture;
static kore_gpu_texture_view test_texture_view;
static kore_gpu_sampler      sampler;
static kore_gpu_buffer       constants;
static everything_set        texture_set;
static kore_gpu_texture      depth_texture;

static bool     first_update = true;
static uint64_t update_index = 0;
static kore_gpu_buffer       mip_buffers[MIP_LEVELS * FACE_COUNT];

static void generate_mip_face(uint32_t *pixels, int size, int face_index, int mip_level) {
    int checker_size = size / 8;
    uint32_t colors[6] = {0xFFFF0000, 0xFF00FF00, 0xFF0000FF, 0xFFFFFF00, 0xFFFF00FF, 0xFF00FFFF};
    uint32_t color = colors[face_index];
    
    // Darker for higher mip levels
    float brightness = 1.0f - (float)mip_level * 0.15f;
    uint8_t r = (uint8_t)((color >> 16) & 0xFF) * brightness;
    uint8_t g = (uint8_t)((color >> 8) & 0xFF) * brightness;
    uint8_t b = (uint8_t)(color & 0xFF) * brightness;
    uint32_t adjusted_color = 0xFF000000 | (r << 16) | (g << 8) | b;
    
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            int cx = x / checker_size;
            int cy = y / checker_size;
            int idx = y * size + x;
            
            if ((cx + cy) % 2 == 0) {
                pixels[idx] = adjusted_color;
            } else {
                pixels[idx] = 0xFF000000;
            }
        }
    }
}

static void update(void *data) {
    float time_val = (float)kore_time();
    
    kore_gpu_texture *framebuffer = kore_gpu_device_get_framebuffer(&device);
    float fb_width = (float)framebuffer->width;
    float fb_height = (float)framebuffer->height;
    
    float fov = 3.14159f / 4.0f;
    float aspect = fb_width / fb_height;
    
    kore_matrix4x4 projection = kore_matrix4x4_perspective(fov, aspect, 0.1f, 100.0f);
    
    float cam_dist = 2.5f;
    kore_float3 cam_pos = {
        sinf(time_val * 0.5f) * cam_dist,
        1.0f,
        cosf(time_val * 0.5f) * cam_dist
    };
    kore_float3 target = {0, 0, 0};
    kore_float3 up = {0, 1, 0};
    kore_matrix4x4 view = kore_matrix4x4_look_at(cam_pos, target, up);
    
    kore_matrix4x4 model = kore_matrix4x4_rotation_y(time_val);
    kore_matrix4x4 model_rot_x = kore_matrix4x4_rotation_x(time_val * 0.7f);
    model = kore_matrix4x4_multiply(&model, &model_rot_x);
    
    kore_matrix4x4 mvp = kore_matrix4x4_identity();
    mvp = kore_matrix4x4_multiply(&mvp, &projection);
    mvp = kore_matrix4x4_multiply(&mvp, &view);
    mvp = kore_matrix4x4_multiply(&mvp, &model);

    constants_type *constants_data = constants_type_buffer_lock(&constants, update_index % KORE_GPU_MAX_FRAMEBUFFERS, 1);
    for (int i = 0; i < 16; i++) {
        constants_data->mvp.m[i] = mvp.m[i];
    }
    constants_type_buffer_unlock(&constants);

    if (first_update) {
        int mip_width = TEX_SIZE;
        int mip_height = TEX_SIZE;
        
        for (int mip = 0; mip < MIP_LEVELS; mip++) {
            for (int face = 0; face < FACE_COUNT; face++) {
                kore_gpu_image_copy_buffer source = {
                    .buffer         = &mip_buffers[mip * FACE_COUNT + face],
                    .bytes_per_row  = kore_gpu_device_align_texture_row_bytes(&device, mip_width * 4),
                    .rows_per_image = mip_height,
                };

                kore_gpu_image_copy_texture destination = {
                    .texture   = &test_texture,
                    .mip_level = mip,
                    .origin_z  = face,
                };

                kore_gpu_command_list_copy_buffer_to_texture(&list, &source, &destination, mip_width, mip_height, 1);
            }
            
            mip_width /= 2;
            mip_height /= 2;
        }
    }

    kore_gpu_render_pass_parameters parameters = {
        .color_attachments_count = 1,
        .color_attachments = {
            {
                .load_op = KORE_GPU_LOAD_OP_CLEAR,
                .clear_value = {
                    .r = 0.1f,
                    .g = 0.1f,
                    .b = 0.2f,
                    .a = 1.0f,
                },
                .texture = {
                    .texture           = framebuffer,
                    .array_layer_count = 1,
                    .mip_level_count   = 1,
                    .format            = kore_gpu_device_framebuffer_format(&device),
                    .dimension         = KORE_GPU_TEXTURE_VIEW_DIMENSION_2D,
                },
            },
        },
        .depth_stencil_attachment = {
            .texture = &depth_texture,
            .depth_load_op = KORE_GPU_LOAD_OP_CLEAR,
            .depth_store_op = KORE_GPU_STORE_OP_STORE,
            .depth_clear_value = 1.0f,
            .stencil_load_op = KORE_GPU_LOAD_OP_LOAD,
            .stencil_store_op = KORE_GPU_STORE_OP_DISCARD,
            .stencil_clear_value = 0,
        },
    };
    kore_gpu_command_list_begin_render_pass(&list, &parameters);

    kong_set_render_pipeline_pipeline(&list);

    kong_set_descriptor_set_everything(&list, &texture_set, update_index % KORE_GPU_MAX_FRAMEBUFFERS);

    kong_set_vertex_buffer_vertex_in(&list, &vertices);

    kore_gpu_command_list_set_index_buffer(&list, &indices, KORE_GPU_INDEX_FORMAT_UINT16, 0);

    kore_gpu_command_list_draw_indexed(&list, 36, 1, 0, 0, 0);

    kore_gpu_command_list_end_render_pass(&list);

    kore_gpu_command_list_present(&list);

    kore_gpu_device_execute_command_list(&device, &list);

    if (first_update) {
        for (int i = 0; i < MIP_LEVELS * FACE_COUNT; i++) {
            kore_gpu_buffer_destroy(&mip_buffers[i]);
        }
        first_update = false;
    }

    update_index += 1;
}

int kickstart(int argc, char **argv) {
    kore_init("Cube Texture Test", width, height, NULL, NULL);
    kore_set_update_callback(update, NULL);

    kore_gpu_device_wishlist wishlist = {0};
    kore_gpu_device_create(&device, &wishlist);

    kong_init(&device);

    kore_gpu_device_create_command_list(&device, KORE_GPU_COMMAND_LIST_TYPE_GRAPHICS, &list);

    int mip_width = TEX_SIZE;
    int mip_height = TEX_SIZE;
    
    for (int mip = 0; mip < MIP_LEVELS; mip++) {
        for (int face = 0; face < FACE_COUNT; face++) {
            uint32_t buffer_size = kore_gpu_device_align_texture_row_bytes(&device, mip_width * 4) * mip_height;
            
            kore_gpu_buffer_parameters buffer_params = {
                .size        = buffer_size,
                .usage_flags = KORE_GPU_BUFFER_USAGE_CPU_WRITE | KORE_GPU_BUFFER_USAGE_COPY_SRC,
            };
            kore_gpu_device_create_buffer(&device, &buffer_params, &mip_buffers[mip * FACE_COUNT + face]);

            uint32_t *ptr = (uint32_t *)kore_gpu_buffer_lock_all(&mip_buffers[mip * FACE_COUNT + face]);
            generate_mip_face(ptr, mip_width, face, mip);
            kore_gpu_buffer_unlock(&mip_buffers[mip * FACE_COUNT + face]);
        }
        
        mip_width /= 2;
        mip_height /= 2;
    }

    kore_gpu_texture_parameters tex_params = {
        .width                 = TEX_SIZE,
        .height                = TEX_SIZE,
        .depth_or_array_layers = FACE_COUNT,
        .mip_level_count       = MIP_LEVELS,
        .sample_count          = 1,
        .dimension             = KORE_GPU_TEXTURE_DIMENSION_2D,
        .format                = KORE_GPU_TEXTURE_FORMAT_RGBA8_UNORM,
        .usage                 = KORE_GPU_TEXTURE_USAGE_COPY_DST | KORE_GPU_TEXTURE_USAGE_SAMPLED,
    };
    kore_gpu_device_create_texture(&device, &tex_params, &test_texture);

    test_texture_view.texture = &test_texture;
    test_texture_view.format = KORE_GPU_TEXTURE_FORMAT_RGBA8_UNORM;
    test_texture_view.dimension = KORE_GPU_TEXTURE_VIEW_DIMENSION_CUBE;
    test_texture_view.aspect = KORE_GPU_IMAGE_COPY_ASPECT_ALL;
    test_texture_view.base_mip_level = 0;
    test_texture_view.mip_level_count = MIP_LEVELS;
    test_texture_view.base_array_layer = 0;
    test_texture_view.array_layer_count = FACE_COUNT;
    kore_gpu_texture_view_create(&device, &test_texture, &test_texture_view);

    kore_gpu_sampler_parameters sampler_params = {
        .address_mode_u = KORE_GPU_ADDRESS_MODE_REPEAT,
        .address_mode_v = KORE_GPU_ADDRESS_MODE_REPEAT,
        .address_mode_w = KORE_GPU_ADDRESS_MODE_REPEAT,
        .mag_filter     = KORE_GPU_FILTER_MODE_LINEAR,
        .min_filter     = KORE_GPU_FILTER_MODE_LINEAR,
        .mipmap_filter  = KORE_GPU_MIPMAP_FILTER_MODE_LINEAR,
        .lod_min_clamp  = 0,
        .lod_max_clamp  = MIP_LEVELS - 1,
        .compare        = KORE_GPU_COMPARE_FUNCTION_UNDEFINED,
        .max_anisotropy = 1,
    };
    kore_gpu_device_create_sampler(&device, &sampler_params, &sampler);

    kong_create_buffer_vertex_in(&device, 24, &vertices);
    vertex_in *v = kong_vertex_in_buffer_lock(&vertices);

    v[0].pos.x = -0.5f; v[0].pos.y = -0.5f; v[0].pos.z =  0.5f;
    v[0].tex.x = 0.0f; v[0].tex.y = 0.0f; v[0].tex.z = 1.0f;
    v[1].pos.x =  0.5f; v[1].pos.y = -0.5f; v[1].pos.z =  0.5f;
    v[1].tex.x = 1.0f; v[1].tex.y = 0.0f; v[1].tex.z = 1.0f;
    v[2].pos.x =  0.5f; v[2].pos.y =  0.5f; v[2].pos.z =  0.5f;
    v[2].tex.x = 1.0f; v[2].tex.y = 1.0f; v[2].tex.z = 1.0f;
    v[3].pos.x = -0.5f; v[3].pos.y =  0.5f; v[3].pos.z =  0.5f;
    v[3].tex.x = 0.0f; v[3].tex.y = 1.0f; v[3].tex.z = 1.0f;

    v[4].pos.x =  0.5f; v[4].pos.y = -0.5f; v[4].pos.z = -0.5f;
    v[4].tex.x = 0.0f; v[4].tex.y = 0.0f; v[4].tex.z = -1.0f;
    v[5].pos.x = -0.5f; v[5].pos.y = -0.5f; v[5].pos.z = -0.5f;
    v[5].tex.x = 1.0f; v[5].tex.y = 0.0f; v[5].tex.z = -1.0f;
    v[6].pos.x = -0.5f; v[6].pos.y =  0.5f; v[6].pos.z = -0.5f;
    v[6].tex.x = 1.0f; v[6].tex.y = 1.0f; v[6].tex.z = -1.0f;
    v[7].pos.x =  0.5f; v[7].pos.y =  0.5f; v[7].pos.z = -0.5f;
    v[7].tex.x = 0.0f; v[7].tex.y = 1.0f; v[7].tex.z = -1.0f;

    v[8].pos.x = -0.5f; v[8].pos.y =  0.5f; v[8].pos.z =  0.5f;
    v[8].tex.x = 0.0f; v[8].tex.y = 1.0f; v[8].tex.z = 0.0f;
    v[9].pos.x =  0.5f; v[9].pos.y =  0.5f; v[9].pos.z =  0.5f;
    v[9].tex.x = 1.0f; v[9].tex.y = 1.0f; v[9].tex.z = 0.0f;
    v[10].pos.x =  0.5f; v[10].pos.y =  0.5f; v[10].pos.z = -0.5f;
    v[10].tex.x = 1.0f; v[10].tex.y = 0.0f; v[10].tex.z = 0.0f;
    v[11].pos.x = -0.5f; v[11].pos.y =  0.5f; v[11].pos.z = -0.5f;
    v[11].tex.x = 0.0f; v[11].tex.y = 0.0f; v[11].tex.z = 0.0f;

    v[12].pos.x = -0.5f; v[12].pos.y = -0.5f; v[12].pos.z = -0.5f;
    v[12].tex.x = 0.0f; v[12].tex.y = 1.0f; v[12].tex.z = 0.0f;
    v[13].pos.x =  0.5f; v[13].pos.y = -0.5f; v[13].pos.z = -0.5f;
    v[13].tex.x = 1.0f; v[13].tex.y = 1.0f; v[13].tex.z = 0.0f;
    v[14].pos.x =  0.5f; v[14].pos.y = -0.5f; v[14].pos.z =  0.5f;
    v[14].tex.x = 1.0f; v[14].tex.y = 0.0f; v[14].tex.z = 0.0f;
    v[15].pos.x = -0.5f; v[15].pos.y = -0.5f; v[15].pos.z =  0.5f;
    v[15].tex.x = 0.0f; v[15].tex.y = 0.0f; v[15].tex.z = 0.0f;

    v[16].pos.x =  0.5f; v[16].pos.y = -0.5f; v[16].pos.z =  0.5f;
    v[16].tex.x = 1.0f; v[16].tex.y = 0.0f; v[16].tex.z = 0.0f;
    v[17].pos.x =  0.5f; v[17].pos.y = -0.5f; v[17].pos.z = -0.5f;
    v[17].tex.x = 0.0f; v[17].tex.y = 0.0f; v[17].tex.z = 0.0f;
    v[18].pos.x =  0.5f; v[18].pos.y =  0.5f; v[18].pos.z = -0.5f;
    v[18].tex.x = 0.0f; v[18].tex.y = 1.0f; v[18].tex.z = 0.0f;
    v[19].pos.x =  0.5f; v[19].pos.y =  0.5f; v[19].pos.z =  0.5f;
    v[19].tex.x = 1.0f; v[19].tex.y = 1.0f; v[19].tex.z = 0.0f;

    v[20].pos.x = -0.5f; v[20].pos.y = -0.5f; v[20].pos.z = -0.5f;
    v[20].tex.x = 1.0f; v[20].tex.y = 0.0f; v[20].tex.z = 0.0f;
    v[21].pos.x = -0.5f; v[21].pos.y = -0.5f; v[21].pos.z =  0.5f;
    v[21].tex.x = 0.0f; v[21].tex.y = 0.0f; v[21].tex.z = 0.0f;
    v[22].pos.x = -0.5f; v[22].pos.y =  0.5f; v[22].pos.z =  0.5f;
    v[22].tex.x = 0.0f; v[22].tex.y = 1.0f; v[22].tex.z = 0.0f;
    v[23].pos.x = -0.5f; v[23].pos.y =  0.5f; v[23].pos.z = -0.5f;
    v[23].tex.x = 1.0f; v[23].tex.y = 1.0f; v[23].tex.z = 0.0f;

    kong_vertex_in_buffer_unlock(&vertices);

    kore_gpu_buffer_parameters index_params = {
        .size        = 36 * sizeof(uint16_t),
        .usage_flags = KORE_GPU_BUFFER_USAGE_INDEX | KORE_GPU_BUFFER_USAGE_CPU_WRITE,
    };
    kore_gpu_device_create_buffer(&device, &index_params, &indices);
    {
        uint16_t *i = (uint16_t *)kore_gpu_buffer_lock_all(&indices);

        i[0] = 0; i[1] = 1; i[2] = 2;
        i[3] = 0; i[4] = 2; i[5] = 3;
        i[6] = 4; i[7] = 5; i[8] = 6;
        i[9] = 4; i[10] = 6; i[11] = 7;
        i[12] = 8; i[13] = 9; i[14] = 10;
        i[15] = 8; i[16] = 10; i[17] = 11;
        i[18] = 12; i[19] = 13; i[20] = 14;
        i[21] = 12; i[22] = 14; i[23] = 15;
        i[24] = 16; i[25] = 17; i[26] = 18;
        i[27] = 16; i[28] = 18; i[29] = 19;
        i[30] = 20; i[31] = 21; i[32] = 22;
        i[33] = 20; i[34] = 22; i[35] = 23;

        kore_gpu_buffer_unlock(&indices);
    }

    kore_gpu_texture_parameters depth_params = {
        .width = width,
        .height = height,
        .depth_or_array_layers = 1,
        .mip_level_count = 1,
        .sample_count = 1,
        .dimension = KORE_GPU_TEXTURE_DIMENSION_2D,
        .format = KORE_GPU_TEXTURE_FORMAT_DEPTH32_FLOAT,
        .usage = KORE_GPU_TEXTURE_USAGE_RENDER_ATTACHMENT,
    };
    kore_gpu_device_create_texture(&device, &depth_params, &depth_texture);

    constants_type_buffer_create(&device, &constants, KORE_GPU_MAX_FRAMEBUFFERS);

    everything_parameters everything_params = {
        .constants = &constants,
        .tex = test_texture_view,
        .sam = &sampler,
    };
    kong_create_everything_set(&device, &everything_params, &texture_set);

    kore_start();

    kore_gpu_texture_destroy(&depth_texture);
    kong_destroy_everything_set(&texture_set);
    constants_type_buffer_destroy(&constants);
    kore_gpu_buffer_destroy(&indices);
    kore_gpu_sampler_destroy(&sampler);
    kong_destroy_buffer_vertex_in(&vertices);
    kore_gpu_texture_view_destroy(&test_texture_view);
    kore_gpu_texture_destroy(&test_texture);
    kore_gpu_command_list_destroy(&list);
    kore_gpu_device_destroy(&device);

    return 0;
}
