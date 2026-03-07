#include <kore3/gpu/device.h>
#include <kore3/gpu/pipeline.h>
#include <kore3/gpu/sampler.h>
#include <kore3/system.h>

#include <kong.h>

#include <stdlib.h>

static const int width  = 800;
static const int height = 600;

static kore_gpu_device       device;
static kore_gpu_command_list list;
static vertex_in_buffer      vertices;
static kore_gpu_buffer       indices;
static kore_gpu_buffer       image_buffer0;
static kore_gpu_buffer       image_buffer1;
static kore_gpu_buffer       image_buffer2;
static kore_gpu_buffer       image_buffer3;
static kore_gpu_texture      texture;
static kore_gpu_texture_view texture_view;
static kore_gpu_sampler      sampler;
static everything_set        texture_set;

static void generate_mip_data(uint32_t *pixels, int size, int mip_level) {
    int checker_size = size / 8;
    uint32_t colors[4] = {0x000000FF, 0x00FF0000, 0x0000FF00, 0x00FFFF00};
    uint32_t color = colors[mip_level];
    
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            int cx = x / checker_size;
            int cy = y / checker_size;
            int idx = y * size + x;
            
            if ((cx + cy) % 2 == 0) {
                pixels[idx] = color;
            } else {
                pixels[idx] = 0xFF000000;
            }
        }
    }
}

static void update(void *data) {
    kore_gpu_texture *framebuffer = kore_gpu_device_get_framebuffer(&device);

    kore_gpu_render_pass_parameters parameters = {
        .color_attachments_count = 1,
        .color_attachments = {
            {
                .load_op = KORE_GPU_LOAD_OP_CLEAR,
                .clear_value = {.r = 0.1f, .g = 0.1f, .b = 0.1f, .a = 1.0f},
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
    kong_set_descriptor_set_everything(&list, &texture_set);
    kong_set_vertex_buffer_vertex_in(&list, &vertices);
    kore_gpu_command_list_set_index_buffer(&list, &indices, KORE_GPU_INDEX_FORMAT_UINT16, 0);
    kore_gpu_command_list_draw_indexed(&list, 3, 1, 0, 0, 0);
    kore_gpu_command_list_end_render_pass(&list);
    kore_gpu_command_list_present(&list);
    kore_gpu_device_execute_command_list(&device, &list);
}

int kickstart(int argc, char **argv) {
    kore_init("Mipmap Test", width, height, NULL, NULL);
    kore_set_update_callback(update, NULL);

    kore_gpu_device_wishlist wishlist = {0};
    kore_gpu_device_create(&device, &wishlist);
    kong_init(&device);
    kore_gpu_device_create_command_list(&device, KORE_GPU_COMMAND_LIST_TYPE_GRAPHICS, &list);

    {
        kore_gpu_buffer_parameters buffer_params = {
            .size = 512 * 512 * 4,
            .usage_flags = KORE_GPU_BUFFER_USAGE_CPU_WRITE | KORE_GPU_BUFFER_USAGE_COPY_SRC,
        };
        kore_gpu_device_create_buffer(&device, &buffer_params, &image_buffer0);
        uint32_t *ptr = (uint32_t *)kore_gpu_buffer_lock_all(&image_buffer0);
        generate_mip_data(ptr, 512, 0);
        kore_gpu_buffer_unlock(&image_buffer0);
    }

    {
        kore_gpu_buffer_parameters buffer_params = {
            .size = 256 * 256 * 4,
            .usage_flags = KORE_GPU_BUFFER_USAGE_CPU_WRITE | KORE_GPU_BUFFER_USAGE_COPY_SRC,
        };
        kore_gpu_device_create_buffer(&device, &buffer_params, &image_buffer1);
        uint32_t *ptr = (uint32_t *)kore_gpu_buffer_lock_all(&image_buffer1);
        generate_mip_data(ptr, 256, 1);
        kore_gpu_buffer_unlock(&image_buffer1);
    }

    {
        kore_gpu_buffer_parameters buffer_params = {
            .size = 128 * 128 * 4,
            .usage_flags = KORE_GPU_BUFFER_USAGE_CPU_WRITE | KORE_GPU_BUFFER_USAGE_COPY_SRC,
        };
        kore_gpu_device_create_buffer(&device, &buffer_params, &image_buffer2);
        uint32_t *ptr = (uint32_t *)kore_gpu_buffer_lock_all(&image_buffer2);
        generate_mip_data(ptr, 128, 2);
        kore_gpu_buffer_unlock(&image_buffer2);
    }

    {
        kore_gpu_buffer_parameters buffer_params = {
            .size = 64 * 64 * 4,
            .usage_flags = KORE_GPU_BUFFER_USAGE_CPU_WRITE | KORE_GPU_BUFFER_USAGE_COPY_SRC,
        };
        kore_gpu_device_create_buffer(&device, &buffer_params, &image_buffer3);
        uint32_t *ptr = (uint32_t *)kore_gpu_buffer_lock_all(&image_buffer3);
        generate_mip_data(ptr, 64, 3);
        kore_gpu_buffer_unlock(&image_buffer3);
    }

    {
        kore_gpu_texture_parameters tex_params = {
            .width = 512, .height = 512, .depth_or_array_layers = 1,
            .mip_level_count = 4, .sample_count = 1,
            .dimension = KORE_GPU_TEXTURE_DIMENSION_2D,
            .format = KORE_GPU_TEXTURE_FORMAT_BGRA8_UNORM,
            .usage = KORE_GPU_TEXTURE_USAGE_COPY_DST | KORE_GPU_TEXTURE_USAGE_SAMPLED,
        };
        kore_gpu_device_create_texture(&device, &tex_params, &texture);
    }

    {
        kore_gpu_image_copy_buffer source = {.bytes_per_row = 512 * 4, .rows_per_image = 512, .buffer = &image_buffer0};
        kore_gpu_image_copy_texture destination = {.mip_level = 0, .texture = &texture};
        kore_gpu_command_list_copy_buffer_to_texture(&list, &source, &destination, 512, 512, 1);
    }

    {
        kore_gpu_image_copy_buffer source = {.bytes_per_row = 256 * 4, .rows_per_image = 256, .buffer = &image_buffer1};
        kore_gpu_image_copy_texture destination = {.mip_level = 1, .texture = &texture};
        kore_gpu_command_list_copy_buffer_to_texture(&list, &source, &destination, 256, 256, 1);
    }

    {
        kore_gpu_image_copy_buffer source = {.bytes_per_row = 128 * 4, .rows_per_image = 128, .buffer = &image_buffer2};
        kore_gpu_image_copy_texture destination = {.mip_level = 2, .texture = &texture};
        kore_gpu_command_list_copy_buffer_to_texture(&list, &source, &destination, 128, 128, 1);
    }

    {
        kore_gpu_image_copy_buffer source = {.bytes_per_row = 64 * 4, .rows_per_image = 64, .buffer = &image_buffer3};
        kore_gpu_image_copy_texture destination = {.mip_level = 3, .texture = &texture};
        kore_gpu_command_list_copy_buffer_to_texture(&list, &source, &destination, 64, 64, 1);
    }

    kore_gpu_texture_view_create(&device, &texture, &texture_view);

    kore_gpu_sampler_parameters sampler_params = {
        .lod_min_clamp = 0, .lod_max_clamp = 3, .max_anisotropy = 1,
    };
    kore_gpu_device_create_sampler(&device, &sampler_params, &sampler);

    kong_create_buffer_vertex_in(&device, 3, &vertices);
    vertex_in *v = kong_vertex_in_buffer_lock(&vertices);
    v[0].pos.x = -1.0f; v[0].pos.y = -1.0f;
    v[1].pos.x = 3.0f; v[1].pos.y = -1.0f;
    v[2].pos.x = -1.0f; v[2].pos.y = 3.0f;
    kong_vertex_in_buffer_unlock(&vertices);

    kore_gpu_buffer_parameters index_params = {
        .size = 3 * sizeof(uint16_t),
        .usage_flags = KORE_GPU_BUFFER_USAGE_INDEX | KORE_GPU_BUFFER_USAGE_CPU_WRITE,
    };
    kore_gpu_device_create_buffer(&device, &index_params, &indices);
    uint16_t *i = (uint16_t *)kore_gpu_buffer_lock_all(&indices);
    i[0] = 0; i[1] = 1; i[2] = 2;
    kore_gpu_buffer_unlock(&indices);

    everything_parameters everything_params = {.tex = texture_view, .sam = &sampler};
    kong_create_everything_set(&device, &everything_params, &texture_set);

    kore_start();

    kore_gpu_buffer_destroy(&image_buffer0);
    kore_gpu_buffer_destroy(&image_buffer1);
    kore_gpu_buffer_destroy(&image_buffer2);
    kore_gpu_buffer_destroy(&image_buffer3);
    kong_destroy_everything_set(&texture_set);
    kore_gpu_buffer_destroy(&indices);
    kong_destroy_buffer_vertex_in(&vertices);
    kore_gpu_sampler_destroy(&sampler);
    kore_gpu_texture_view_destroy(&texture_view);
    kore_gpu_texture_destroy(&texture);
    kore_gpu_command_list_destroy(&list);
    kore_gpu_device_destroy(&device);

    return 0;
}
