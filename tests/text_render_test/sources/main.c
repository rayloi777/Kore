#include <kore3/text.h>
#include <kore3/gpu/device.h>
#include <kore3/system.h>

#include <kong.h>

#include <stdlib.h>
#include <stdio.h>

static const int width = 800;
static const int height = 600;

static kore_gpu_device device;
static kore_gpu_command_list list;
static kore_text_font *font;

static int basic_glyphs[] = {
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
    64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
    96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
    112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126,
    0x4E2D, 0x6587, 0x4F60, 0x597D, 0x4E16, 0x754C, 0x0
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
                    .format = kore_gpu_device_framebuffer_format(&device),
                    .dimension = KORE_GPU_TEXTURE_VIEW_DIMENSION_2D,
                },
            },
        },
    };
    kore_gpu_command_list_begin_render_pass(&list, &parameters);
    
    if (font) {
        kore_text_draw(font, "Hello World!", 50.0f, 100.0f, 1.0f, 0.0f, 0.0f, 1.0f);
        kore_text_draw(font, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 50.0f, 150.0f, 0.0f, 1.0f, 0.0f, 1.0f);
        kore_text_draw(font, "abcdefghijklmnopqrstuvwxyz", 50.0f, 200.0f, 0.0f, 0.0f, 1.0f, 1.0f);
        kore_text_draw(font, "0123456789", 50.0f, 250.0f, 1.0f, 1.0f, 0.0f, 1.0f);
        
        float w = kore_text_width(font, "Hello World!");
        char buf[64];
        snprintf(buf, sizeof(buf), "Width: %.1f", w);
        kore_text_draw(font, buf, 50.0f, 300.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    }
    
    kore_gpu_command_list_end_render_pass(&list);
    kore_gpu_command_list_present(&list);
    kore_gpu_device_execute_command_list(&device, &list);
}

int kickstart(int argc, char **argv) {
    kore_init("Text Render Test", width, height, NULL, NULL);
    kore_set_update_callback(update, NULL);

    kore_gpu_device_wishlist wishlist = {0};
    kore_gpu_device_create(&device, &wishlist);

    kong_init(&device);

    kore_gpu_device_create_command_list(&device, KORE_GPU_COMMAND_LIST_TYPE_GRAPHICS, &list);
    
    kore_text_init(&device);
    
    font = kore_text_font_create("deployment/NotoSansTC-Regular.ttf", basic_glyphs, sizeof(basic_glyphs) / sizeof(basic_glyphs[0]), 24.0f);
    
    if (font) {
        printf("[text_render_test] Font loaded successfully\n");
    } else {
        printf("[text_render_test] Font failed to load\n");
    }

    kore_start();

    if (font) {
        kore_text_font_destroy(font);
    }
    kore_gpu_command_list_destroy(&list);
    kore_gpu_device_destroy(&device);

    return 0;
}
