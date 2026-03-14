#include <kore3/text.h>
#include <kore3/gpu/device.h>
#include <kore3/gpu/commandlist.h>
#include <kore3/system.h>
#include <kore3/log.h>

#include <kong.h>

#include <stdlib.h>
#include <stdio.h>

void draw_begin(void);

static const int width = 800;
static const int height = 600;

static kore_gpu_device device;
static kore_gpu_command_list list;
static draw_font *font_small;
static draw_font *font_medium;
static draw_font *font_large;
static kore_gpu_texture depth_texture;

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
    };
    kore_gpu_command_list_begin_render_pass(&list, &parameters);
    
    kore_gpu_command_list_set_viewport(&list, 0, 0, (float)framebuffer->width, (float)framebuffer->height, 0.0f, 1.0f);
    
    draw_begin();
    
    if (font_small) {
        draw_string(font_small, "Small 24px", 50.0f, 100.0f, 1.0f, 0.0f, 0.0f, 1.0f);
    }
    if (font_medium) {
        draw_string(font_medium, "Medium 48px", 50.0f, 200.0f, 0.0f, 1.0f, 0.0f, 1.0f);
    }
    if (font_large) {
        draw_string(font_large, "Large 72px", 50.0f, 350.0f, 0.0f, 0.0f, 1.0f, 1.0f);
    }
    
    kore_gpu_command_list_end_render_pass(&list);
    kore_gpu_device_execute_command_list(&device, &list);
    kore_gpu_command_list_present(&list);
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
    
    font_small = draw_font_create("NotoSansTC-Regular.ttf", (int *)kore_basic_glyphs, KORE_BASIC_GLYPHS_COUNT, 24.0f);
    font_medium = draw_font_create("NotoSansTC-Regular.ttf", (int *)kore_basic_glyphs, KORE_BASIC_GLYPHS_COUNT, 48.0f);
    font_large = draw_font_create("NotoSansTC-Regular.ttf", (int *)kore_basic_glyphs, KORE_BASIC_GLYPHS_COUNT, 72.0f);
    
    fprintf(stderr, "Fonts: small=%p medium=%p large=%p\n", 
            (void*)font_small, (void*)font_medium, (void*)font_large);
    
    if (font_small) kore_log(KORE_LOG_LEVEL_INFO, "Font small loaded");
    if (font_medium) kore_log(KORE_LOG_LEVEL_INFO, "Font medium loaded");
    if (font_large) kore_log(KORE_LOG_LEVEL_INFO, "Font large loaded");

    kore_start();

    if (font_small) draw_font_destroy(font_small);
    if (font_medium) draw_font_destroy(font_medium);
    if (font_large) draw_font_destroy(font_large);
    kore_gpu_command_list_destroy(&list);
    kore_gpu_device_destroy(&device);

    return 0;
}
