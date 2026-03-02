#include <kore3/gpu/device.h>
#include <kore3/g2/g2.h>
#include <kore3/g2/font.h>
#include <kore3/system.h>
#include <kore3/window.h>
#include <string.h>
#include <stdio.h>

static kore_gpu_device device;
static kore_gpu_command_list list;
static kore_g2_font font;
static int width = 1024;
static int height = 768;

static void update(void *data) {
	(void)data;
	
	kore_gpu_device_create_command_list(&device, KORE_GPU_COMMAND_LIST_TYPE_GRAPHICS, &list);
	
	kore_gpu_texture *framebuffer = kore_gpu_device_get_framebuffer(&device);

kore_gpu_color clear_color = {
.r = 0.1f,
.g = 0.1f,
.b = 0.1f,
.a = 1.0f,
};

kore_gpu_render_pass_parameters pass_params = {0};
pass_params.color_attachments_count = 1;
pass_params.color_attachments[0].load_op = KORE_GPU_LOAD_OP_CLEAR;
pass_params.color_attachments[0].clear_value = clear_color;
pass_params.color_attachments[0].texture.texture = framebuffer;
pass_params.color_attachments[0].texture.array_layer_count = 1;
pass_params.color_attachments[0].texture.mip_level_count = 1;
pass_params.color_attachments[0].texture.format = KORE_GPU_TEXTURE_FORMAT_BGRA8_UNORM;
pass_params.color_attachments[0].texture.dimension = KORE_GPU_TEXTURE_VIEW_DIMENSION_2D;

kore_gpu_command_list_begin_render_pass(&list, &pass_params);

kore_g2_begin(&list);
kore_g2_set_font(&font);
kore_g2_set_color(1.0f, 1.0f, 1.0f, 1.0f);
kore_g2_draw_string("Traditional Chinese:", 50.0f, 50.0f);
kore_g2_draw_string("繁體中文測試", 50.0f, 100.0f);
kore_g2_draw_string("繁體中文", 50.0f, 150.0f);
kore_g2_draw_string("測試成功", 50.0f, 200.0f);
kore_g2_draw_string("English Text:", 50.0f, 300.0f);
kore_g2_draw_string("Hello World!", 50.0f, 350.0f);
kore_g2_draw_string("1234567890", 50.0f, 400.0f);
kore_g2_end();

kore_gpu_command_list_end_render_pass(&list);
kore_gpu_command_list_present(&list);
kore_gpu_device_execute_command_list(&device, &list);
}

int kickstart(int argc, char **argv) {
	(void)argc;
	(void)argv;
	
	kore_init("Text Test", width, height, NULL, NULL);
	kore_set_update_callback(update, NULL);
	
	kore_gpu_device_wishlist wishlist = {0};
	kore_gpu_device_create(&device, &wishlist);
	
	kore_g2_init(&device, width, height);
	
	int glyph_count;
	const int *glyphs = kore_g2_font_get_traditional_chinese_glyphs(&glyph_count);
	
	if (!kore_g2_font_init_with_glyphs(&font, &device, "NotoSansTC-Regular.ttf", 32.0f, glyphs, glyph_count)) {
		kore_log(KORE_LOG_LEVEL_ERROR, "Failed to load font");
	}
	
	kore_start();
	
	kore_g2_font_destroy(&font);
	
	return 0;
}
