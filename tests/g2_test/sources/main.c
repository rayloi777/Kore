#include <kore3/gpu/device.h>
#include <kore3/gpu/buffer.h>
#include <kore3/gpu/pipeline.h>
#include <kore3/gpu/texture.h>
#include <kore3/math/matrix.h>
#include <kore3/system.h>

#include <kong.h>

#include <stdlib.h>
#include <stdio.h>

static const int width  = 1920;
static const int height = 1080;

static kore_gpu_device       device;
static kore_gpu_command_list list;
static vertex_in_buffer      vertices;
static kore_gpu_buffer       indices;
static kore_gpu_buffer       constants;
static kore_gpu_texture      dummy_texture;
static kore_gpu_texture_view dummy_texture_view;
static kore_gpu_sampler      dummy_sampler;
static everything_set        test_set;

static uint64_t update_index = 0;

static void update(void *data) {
    kore_gpu_texture *framebuffer = kore_gpu_device_get_framebuffer(&device);

    kore_gpu_render_pass_parameters parameters = {
        .color_attachments_count = 1,
        .color_attachments = {
            {
                .load_op = KORE_GPU_LOAD_OP_CLEAR,
                .clear_value = {.r = 0.1f, .g = 0.1f, .b = 0.2f, .a = 1.0f},
                .texture = {
                    .texture = framebuffer,
                    .array_layer_count = 1,
                    .mip_level_count = 1,
                    .format = kore_gpu_device_framebuffer_format(&device),
                    .dimension = KORE_GPU_TEXTURE_VIEW_DIMENSION_2D,
                },
            },
        },
    };
    kore_gpu_command_list_begin_render_pass(&list, &parameters);
    
    kong_set_render_pipeline_pipeline(&list);
    
    constants_type *constants_data = constants_type_buffer_lock(&constants, update_index % KORE_GPU_MAX_FRAMEBUFFERS, 1);
    
    kore_matrix3x3 mvp = kore_matrix3x3_identity();
    mvp.m[0] = 2.0f / width;
    mvp.m[1] = 0.0f;
    mvp.m[2] = -1.0f;
    
    mvp.m[3] = 0.0f;
    mvp.m[4] = 2.0f / height;
    mvp.m[5] = -1.0f;
    
    mvp.m[6] = 0.0f;
    mvp.m[7] = 0.0f;
    mvp.m[8] = 1.0f;
    constants_data->mvp = mvp;
    
    constants_type_buffer_unlock(&constants);
    
    kong_set_descriptor_set_everything(&list, &test_set, update_index % KORE_GPU_MAX_FRAMEBUFFERS);
    
    kong_set_vertex_buffer_vertex_in(&list, &vertices);
    
    kore_gpu_command_list_set_index_buffer(&list, &indices, KORE_GPU_INDEX_FORMAT_UINT16, 0);
    
    kore_gpu_command_list_draw_indexed(&list, 6, 1, 0, 0, 0);
    
    kore_gpu_command_list_end_render_pass(&list);
    kore_gpu_command_list_present(&list);
    kore_gpu_device_execute_command_list(&device, &list);
    
    update_index += 1;
}

int kickstart(int argc, char **argv) {
    kore_init("Solid Color Test", width, height, NULL, NULL);
    kore_set_update_callback(update, NULL);

    kore_gpu_device_wishlist wishlist = {0};
    kore_gpu_device_create(&device, &wishlist);

    kong_init(&device);

    kore_gpu_device_create_command_list(&device, KORE_GPU_COMMAND_LIST_TYPE_GRAPHICS, &list);

    kong_create_buffer_vertex_in(&device, 4, &vertices);
    {
        vertex_in *v = kong_vertex_in_buffer_lock(&vertices);
        
        v[0].pos.x = 100.0f;
        v[0].pos.y = 100.0f;
        
        v[1].pos.x = 300.0f;
        v[1].pos.y = 100.0f;
        
        v[2].pos.x = 300.0f;
        v[2].pos.y = 300.0f;
        
        v[3].pos.x = 100.0f;
        v[3].pos.y = 300.0f;
        v[3].color.x = 1.0f;
        v[3].color.y = 0.0f;
        v[3].color.z = 0.0f;
        v[3].color.w = 1.0f;
        
        kong_vertex_in_buffer_unlock(&vertices);
    }
    
    {
        kore_gpu_buffer_parameters params = {
            .size = 6 * sizeof(uint16_t),
            .usage_flags = KORE_GPU_BUFFER_USAGE_INDEX | KORE_GPU_BUFFER_USAGE_CPU_WRITE,
        };
        kore_gpu_device_create_buffer(&device, &params, &indices);
        uint16_t *i = (uint16_t *)kore_gpu_buffer_lock_all(&indices);
        i[0] = 0;
        i[1] = 1;
        i[2] = 2;
        i[3] = 0;
        i[4] = 2;
        i[5] = 3;
        kore_gpu_buffer_unlock(&indices);
    }
    
    constants_type_buffer_create(&device, &constants, KORE_GPU_MAX_FRAMEBUFFERS);
    
    {
        kore_gpu_texture_parameters params = {
            .width = 1,
            .height = 1,
            .depth_or_array_layers = 1,
            .mip_level_count = 1,
            .sample_count = 1,
            .dimension = KORE_GPU_TEXTURE_DIMENSION_2D,
            .format = KORE_GPU_TEXTURE_FORMAT_RGBA8_UNORM,
            .usage = KORE_GPU_TEXTURE_USAGE_SAMPLED,
        };
        kore_gpu_device_create_texture(&device, &params, &dummy_texture);
    }
    kore_gpu_texture_view_create(&device, &dummy_texture, &dummy_texture_view);
    
    {
        kore_gpu_sampler_parameters params = {
            .address_mode_u = KORE_GPU_ADDRESS_MODE_CLAMP_TO_EDGE,
            .address_mode_v = KORE_GPU_ADDRESS_MODE_CLAMP_TO_EDGE,
            .mag_filter = KORE_GPU_FILTER_MODE_NEAREST,
            .min_filter = KORE_GPU_FILTER_MODE_NEAREST,
        };
        kore_gpu_device_create_sampler(&device, &params, &dummy_sampler);
    }
    
    {
        everything_parameters params = {
            .constants = &constants,
            .tex = dummy_texture_view,
            .sam = &dummy_sampler,
        };
        kong_create_everything_set(&device, &params, &test_set);
    }

    kore_start();

    kong_destroy_everything_set(&test_set);
    kore_gpu_sampler_destroy(&dummy_sampler);
    kore_gpu_texture_destroy(&dummy_texture);
    constants_type_buffer_destroy(&constants);
    kore_gpu_buffer_destroy(&indices);
    kong_destroy_buffer_vertex_in(&vertices);
    kore_gpu_command_list_destroy(&list);
    kore_gpu_device_destroy(&device);

    return 0;
}
