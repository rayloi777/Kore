#include <kore3/text.h>
#include <kore3/gpu/device.h>
#include <kore3/gpu/commandlist.h>
#include <kore3/system.h>
#include <kore3/log.h>

#include <kong.h>

#include <stdlib.h>
#include <stdio.h>

static const int width = 800;
static const int height = 600;

static kore_gpu_device device;
static kore_gpu_command_list list;
static draw_font *font;
static kore_gpu_texture depth_texture;

static int basic_glyphs[] = {
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
    64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
    96, 97, 98, 99,
};

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
                    .format = KORE_GPU_TEXTURE_FORMAT_BGRA8_UNORM,
                    .dimension = KORE_GPU_TEXTURE_VIEW_DIMENSION_2D,
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
    
    kore_gpu_command_list_set_viewport(&list, 0, 0, (float)framebuffer->width, (float)framebuffer->height, 0.0f, 1.0f);
    
    if (font) {
        draw_string(font, "Hello World!", 50.0f, 100.0f, 1.0f, 1.0f, 1.0f, 1.0f);
        draw_string(font, "ABC", 50.0f, 150.0f, 1.0f, 0.0f, 0.0f, 1.0f);
        draw_string(font, "abc", 50.0f, 200.0f, 0.0f, 1.0f, 0.0f, 1.0f);
        draw_string(font, "0123", 50.0f, 250.0f, 1.0f, 1.0f, 0.0f, 1.0f);
        
        float w = draw_string_width(font, "Hello World!");
        char buf[64];
        snprintf(buf, sizeof(buf), "Width: %.1f", w);
        draw_string(font, buf, 50.0f, 300.0f, 1.0f, 0.5f, 0.0f, 1.0f);
    }
    
    kore_gpu_command_list_end_render_pass(&list);
    kore_gpu_command_list_present(&list);
    kore_gpu_device_execute_command_list(&device, &list);
}

int kickstart(int argc, char **argv) {
    kore_init("Draw Test", width, height, NULL, NULL);
    kore_set_update_callback(update, NULL);

    kore_gpu_device_wishlist wishlist = {0};
    kore_gpu_device_create(&device, &wishlist);

    kong_init(&device);

    kore_gpu_device_create_command_list(&device, KORE_GPU_COMMAND_LIST_TYPE_GRAPHICS, &list);
    
    draw_init(&device, &list);
    
    kore_gpu_texture *fb = kore_gpu_device_get_framebuffer(&device);
    draw_set_viewport(fb->width, fb->height);
    
    kore_gpu_texture_parameters depth_params = {
        .width = fb->width,
        .height = fb->height,
        .depth_or_array_layers = 1,
        .mip_level_count = 1,
        .sample_count = 1,
        .dimension = KORE_GPU_TEXTURE_DIMENSION_2D,
        .format = KORE_GPU_TEXTURE_FORMAT_DEPTH32_FLOAT,
        .usage = KORE_GPU_TEXTURE_USAGE_RENDER_ATTACHMENT,
    };
    kore_gpu_device_create_texture(&device, &depth_params, &depth_texture);
    
    font = draw_font_create("NotoSansTC-Regular.ttf", basic_glyphs, sizeof(basic_glyphs) / sizeof(basic_glyphs[0]), 64.0f);
    
    fprintf(stderr, "Font create returned: %p\n", (void*)font);
    
    if (font) {
        kore_log(KORE_LOG_LEVEL_INFO, "Font loaded successfully");
    } else {
        kore_log(KORE_LOG_LEVEL_ERROR, "Font failed to load");
    }

    kore_start();

    if (font) {
        draw_font_destroy(font);
    }
    kore_gpu_command_list_destroy(&list);
    kore_gpu_device_destroy(&device);

    return 0;
}
