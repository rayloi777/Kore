#include <kore3/g2/g2.h>
#include <kore3/gpu/device.h>
#include <kore3/gpu/texture.h>
#include <kore3/gpu/buffer.h>
#include <kore3/system.h>

#include <kong.h>

#include <stdlib.h>
#include <stdio.h>

static const int width  = 800;
static const int height = 600;

static kore_gpu_device       device;
static kore_gpu_command_list list;
static kore_gpu_texture      white_texture;
static kore_gpu_texture_view white_texture_view;

static uint64_t update_index = 0;

static void update(void *data) {
    (void)data;
    
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
    
    kore_g2_begin(&list);
    
    kore_g2_set_color(1.0f, 1.0f, 1.0f, 1.0f);
    kore_g2_draw_scaled_image(&white_texture, 100.0f, 100.0f, 200.0f, 200.0f);
    
    kore_g2_set_color(1.0f, 0.0f, 0.0f, 1.0f);
    kore_g2_draw_scaled_image(&white_texture, 350.0f, 100.0f, 150.0f, 150.0f);
    
    kore_g2_set_color(0.0f, 1.0f, 0.0f, 1.0f);
    kore_g2_draw_scaled_image(&white_texture, 100.0f, 350.0f, 100.0f, 100.0f);
    
    kore_g2_flush();
    kore_g2_end();
    
    kore_gpu_command_list_end_render_pass(&list);
    kore_gpu_command_list_present(&list);
    kore_gpu_device_execute_command_list(&device, &list);
    
    update_index += 1;
}

int kickstart(int argc, char **argv) {
    kore_init("g2 Test", width, height, NULL, NULL);
    kore_set_update_callback(update, NULL);

    kore_gpu_device_wishlist wishlist = {0};
    kore_gpu_device_create(&device, &wishlist);

    kong_init(&device);

    kore_gpu_device_create_command_list(&device, KORE_GPU_COMMAND_LIST_TYPE_GRAPHICS, &list);
    
    kore_g2_init(&device, width, height);

    kore_gpu_buffer white_buffer;
    kore_gpu_buffer_parameters buffer_params = {
        .size        = 4,
        .usage_flags = KORE_GPU_BUFFER_USAGE_CPU_WRITE | KORE_GPU_BUFFER_USAGE_COPY_SRC,
    };
    kore_gpu_device_create_buffer(&device, &buffer_params, &white_buffer);
    
    uint8_t *pixels = (uint8_t *)kore_gpu_buffer_lock_all(&white_buffer);
    pixels[0] = 255;
    pixels[1] = 255;
    pixels[2] = 255;
    pixels[3] = 255;
    kore_gpu_buffer_unlock(&white_buffer);

    kore_gpu_texture_parameters tex_params = {
        .width                 = 1,
        .height                = 1,
        .depth_or_array_layers = 1,
        .mip_level_count       = 1,
        .sample_count          = 1,
        .dimension             = KORE_GPU_TEXTURE_DIMENSION_2D,
        .format                = KORE_GPU_TEXTURE_FORMAT_RGBA8_UNORM,
        .usage                 = KORE_GPU_TEXTURE_USAGE_COPY_DST | KORE_GPU_TEXTURE_USAGE_SAMPLED,
    };
    kore_gpu_device_create_texture(&device, &tex_params, &white_texture);

    kore_gpu_image_copy_buffer source = {
        .buffer         = &white_buffer,
        .bytes_per_row  = 4,
        .rows_per_image = 1,
    };

    kore_gpu_image_copy_texture destination = {
        .texture   = &white_texture,
        .mip_level = 0,
    };

    kore_gpu_command_list_copy_buffer_to_texture(&list, &source, &destination, 1, 1, 1);
    kore_gpu_device_execute_command_list(&device, &list);
    
    kore_gpu_buffer_destroy(&white_buffer);
    
    kore_gpu_texture_view_create(&device, &white_texture, &white_texture_view);

    kore_start();

    kore_gpu_texture_view_destroy(&white_texture_view);
    kore_gpu_texture_destroy(&white_texture);
    kore_gpu_command_list_destroy(&list);
    kore_gpu_device_destroy(&device);

    return 0;
}
