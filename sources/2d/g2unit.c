#include <kore3/g2/g2.h>
#include <kore3/gpu/buffer.h>
#include <kore3/gpu/pipeline.h>
#include <kore3/gpu/sampler.h>
#include <kore3/math/matrix.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUFFER_SIZE 1000
#define MAX_VERTICES (MAX_BUFFER_SIZE * 4)
#define MAX_INDICES (MAX_BUFFER_SIZE * 6)

static kore_gpu_device *g_device = NULL;
static int g_screen_width = 0;
static int g_screen_height = 0;
static kore_gpu_command_list *g_command_list = NULL;

static kore_g2_font *g_font = NULL;
static float g_font_size = 16.0f;
static float g_color_r = 1.0f, g_color_g = 1.0f, g_color_b = 1.0f, g_color_a = 1.0f;
static float g_opacity = 1.0f;

static kore_matrix4x4 g_projection;
static kore_matrix4x4 g_transform;
static kore_matrix4x4 g_transform_stack[32];
static int g_transform_index = 0;

static float g_opacity_stack[32];
static int g_opacity_index = 0;

static uint32_t utf8_decode(const char **text) {
	const uint8_t *str = (const uint8_t *)*text;
	uint32_t codepoint = 0;
	
	if (str[0] < 0x80) {
		codepoint = str[0];
		*text += 1;
	}
	else if ((str[0] & 0xE0) == 0xC0) {
		codepoint = ((str[0] & 0x1F) << 6) | (str[1] & 0x3F);
		*text += 2;
	}
	else if ((str[0] & 0xF0) == 0xE0) {
		codepoint = ((str[0] & 0x0F) << 12) | ((str[1] & 0x3F) << 6) | (str[2] & 0x3F);
		*text += 3;
	}
	else if ((str[0] & 0xF8) == 0xF0) {
		codepoint = ((str[0] & 0x07) << 18) | ((str[1] & 0x3F) << 12) | ((str[2] & 0x3F) << 6) | (str[3] & 0x3F);
		*text += 4;
	}
	else {
		*text += 1;
	}
	
	return codepoint;
}

void kore_g2_init(kore_gpu_device *device, int screen_width, int screen_height) {
	g_device = device;
	g_screen_width = screen_width;
	g_screen_height = screen_height;
	
	g_projection = kore_matrix4x4_identity();
	g_transform = kore_matrix4x4_identity();
	g_opacity = 1.0f;
}

void kore_g2_begin(kore_gpu_command_list *list) {
	g_command_list = list;
}

void kore_g2_flush(void) {
}

void kore_g2_end(void) {
}

void kore_g2_clear(float r, float g, float b, float a) {
	(void)r;
	(void)g;
	(void)b;
	(void)a;
}

void kore_g2_draw_image(kore_gpu_texture *texture, float x, float y) {
	(void)texture;
	(void)x;
	(void)y;
}

void kore_g2_draw_scaled_image(kore_gpu_texture *texture, float dx, float dy, float dw, float dh) {
	(void)texture;
	(void)dx;
	(void)dy;
	(void)dw;
	(void)dh;
}

void kore_g2_draw_sub_image(kore_gpu_texture *texture, float x, float y, float sx, float sy, float sw, float sh) {
	(void)texture;
	(void)x;
	(void)y;
	(void)sx;
	(void)sy;
	(void)sw;
	(void)sh;
}

void kore_g2_draw_scaled_sub_image(kore_gpu_texture *texture, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh) {
	(void)texture;
	(void)sx;
	(void)sy;
	(void)sw;
	(void)sh;
	(void)dx;
	(void)dy;
	(void)dw;
	(void)dh;
}

void kore_g2_set_font(kore_g2_font *font) {
	g_font = font;
}

void kore_g2_set_font_size(float size) {
	g_font_size = size;
}

void kore_g2_set_color(float r, float g, float b, float a) {
	g_color_r = r;
	g_color_g = g;
	g_color_b = b;
	g_color_a = a;
}

void kore_g2_set_color_uint(uint32_t color) {
	g_color_r = ((color >> 16) & 0xFF) / 255.0f;
	g_color_g = ((color >> 8) & 0xFF) / 255.0f;
	g_color_b = (color & 0xFF) / 255.0f;
	g_color_a = ((color >> 24) & 0xFF) / 255.0f;
}

void kore_g2_draw_string(const char *utf8_text, float x, float y) {
	(void)utf8_text;
	(void)x;
	(void)y;
}

float kore_g2_string_width(const char *utf8_text) {
	(void)utf8_text;
	return 0.0f;
}

void kore_g2_translate(float tx, float ty) {
	(void)tx;
	(void)ty;
}

void kore_g2_rotate(float angle, float cx, float cy) {
	(void)angle;
	(void)cx;
	(void)cy;
}

void kore_g2_scale(float sx, float sy) {
	(void)sx;
	(void)sy;
}

void kore_g2_push_transformation(void) {
}

void kore_g2_pop_transformation(void) {
}

void kore_g2_set_opacity(float opacity) {
	(void)opacity;
}

void kore_g2_push_opacity(float opacity) {
	(void)opacity;
}

float kore_g2_pop_opacity(void) {
	return 1.0f;
}

void kore_g2_scissor(int x, int y, int width, int height) {
	(void)x;
	(void)y;
	(void)width;
	(void)height;
}

void kore_g2_disable_scissor(void) {
}
