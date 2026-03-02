#include <kore3/g2/g2.h>
#include <kore3/gpu/buffer.h>
#include <kore3/gpu/pipeline.h>
#include <kore3/gpu/sampler.h>
#include <kore3/math/matrix.h>
#include <kong.h>
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

static vertex_in_buffer g_vertex_buffer;
static kore_gpu_buffer g_index_buffer;
static int g_vertex_index = 0;

static kore_gpu_sampler g_sampler;
static textures_set g_textures_set;
static bool g_textures_set_initialized = false;
static kore_g2_font *g_last_font = NULL;

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
	
	kong_init(device);
	
	g_projection = kore_matrix4x4_identity();
	g_transform = kore_matrix4x4_identity();
	g_opacity = 1.0f;
	
	kong_create_buffer_vertex_in(device, MAX_VERTICES, &g_vertex_buffer);
	
	kore_gpu_buffer_parameters index_params = {
		.size = MAX_INDICES * sizeof(uint32_t),
		.usage_flags = KORE_GPU_BUFFER_USAGE_INDEX | KORE_GPU_BUFFER_USAGE_COPY_DST,
	};
	kore_gpu_device_create_buffer(device, &index_params, &g_index_buffer);
	
	uint32_t *indices = kore_gpu_buffer_lock_all(&g_index_buffer);
	for (int i = 0; i < MAX_BUFFER_SIZE; i++) {
		indices[i * 6 + 0] = i * 4 + 0;
		indices[i * 6 + 1] = i * 4 + 1;
		indices[i * 6 + 2] = i * 4 + 2;
		indices[i * 6 + 3] = i * 4 + 0;
		indices[i * 6 + 4] = i * 4 + 2;
		indices[i * 6 + 5] = i * 4 + 3;
	}
	kore_gpu_buffer_unlock_all(&g_index_buffer);
	
	kore_gpu_sampler_parameters sampler_params = {
		.address_mode_u = KORE_GPU_ADDRESS_MODE_CLAMP_TO_EDGE,
		.address_mode_v = KORE_GPU_ADDRESS_MODE_CLAMP_TO_EDGE,
		.address_mode_w = KORE_GPU_ADDRESS_MODE_CLAMP_TO_EDGE,
		.min_filter = KORE_GPU_FILTER_MODE_LINEAR,
		.mag_filter = KORE_GPU_FILTER_MODE_LINEAR,
		.mipmap_filter = KORE_GPU_MIPMAP_FILTER_MODE_NEAREST,
		.max_anisotropy = 1,
	};
	kore_gpu_device_create_sampler(device, &sampler_params, &g_sampler);
}

static void update_textures_set(void) {
	if (!g_font) return;
	
	if (g_last_font != g_font || !g_textures_set_initialized) {
		if (g_textures_set_initialized) {
			kong_destroy_textures_set(&g_textures_set);
		}
		
		textures_parameters params = {
			.tex = {
				.texture = &g_font->texture,
				.format = KORE_GPU_TEXTURE_FORMAT_R8_UNORM,
				.dimension = KORE_GPU_TEXTURE_VIEW_DIMENSION_2D,
				.base_mip_level = 0,
				.mip_level_count = 1,
				.base_array_layer = 0,
				.array_layer_count = 1,
			},
			.sam = &g_sampler,
		};
		kong_create_textures_set(g_device, &params, &g_textures_set);
		g_textures_set_initialized = true;
		g_last_font = g_font;
	}
}

void kore_g2_begin(kore_gpu_command_list *list) {
	g_command_list = list;
	g_vertex_index = 0;
}

static vertex_in *g_locked_vertices = NULL;

void kore_g2_flush(void) {
	if (g_vertex_index == 0) return;
	
	if (g_locked_vertices) {
		kong_vertex_in_buffer_unlock(&g_vertex_buffer);
		g_locked_vertices = NULL;
	}
	
	kong_set_render_pipeline_text_pipeline(g_command_list);
	
	update_textures_set();
	kong_set_descriptor_set_textures(g_command_list, &g_textures_set);
	
	kong_set_vertex_buffer_vertex_in(g_command_list, &g_vertex_buffer);
	kore_gpu_command_list_set_index_buffer(g_command_list, &g_index_buffer, KORE_GPU_INDEX_FORMAT_UINT32, 0);
	kore_gpu_command_list_draw_indexed(g_command_list, g_vertex_index * 6, 1, 0, 0, 0);
	
	g_vertex_index = 0;
}

void kore_g2_end(void) {
	kore_g2_flush();
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
	if (g_font != font) {
		kore_g2_flush();
	}
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

static void add_text_quad(float x, float y, float x1, float y1, float s0, float t0, float s1, float t1) {
	if (g_vertex_index + 1 >= MAX_BUFFER_SIZE) {
		kore_g2_flush();
	}
	
	if (!g_locked_vertices) {
		g_locked_vertices = kong_vertex_in_buffer_lock(&g_vertex_buffer);
	}
	
	int base = g_vertex_index * 4;
	
	g_locked_vertices[base + 0].pos.x = x;
	g_locked_vertices[base + 0].pos.y = y;
	g_locked_vertices[base + 0].pos.z = -5.0f;
	g_locked_vertices[base + 0].tex_coord.x = s0;
	g_locked_vertices[base + 0].tex_coord.y = t0;
	g_locked_vertices[base + 0].col.x = g_color_r;
	g_locked_vertices[base + 0].col.y = g_color_g;
	g_locked_vertices[base + 0].col.z = g_color_b;
	g_locked_vertices[base + 0].col.w = g_color_a * g_opacity;
	
	g_locked_vertices[base + 1].pos.x = x1;
	g_locked_vertices[base + 1].pos.y = y;
	g_locked_vertices[base + 1].pos.z = -5.0f;
	g_locked_vertices[base + 1].tex_coord.x = s1;
	g_locked_vertices[base + 1].tex_coord.y = t0;
	g_locked_vertices[base + 1].col.x = g_color_r;
	g_locked_vertices[base + 1].col.y = g_color_g;
	g_locked_vertices[base + 1].col.z = g_color_b;
	g_locked_vertices[base + 1].col.w = g_color_a * g_opacity;
	
	g_locked_vertices[base + 2].pos.x = x1;
	g_locked_vertices[base + 2].pos.y = y1;
	g_locked_vertices[base + 2].pos.z = -5.0f;
	g_locked_vertices[base + 2].tex_coord.x = s1;
	g_locked_vertices[base + 2].tex_coord.y = t1;
	g_locked_vertices[base + 2].col.x = g_color_r;
	g_locked_vertices[base + 2].col.y = g_color_g;
	g_locked_vertices[base + 2].col.z = g_color_b;
	g_locked_vertices[base + 2].col.w = g_color_a * g_opacity;
	
	g_locked_vertices[base + 3].pos.x = x;
	g_locked_vertices[base + 3].pos.y = y1;
	g_locked_vertices[base + 3].pos.z = -5.0f;
	g_locked_vertices[base + 3].tex_coord.x = s0;
	g_locked_vertices[base + 3].tex_coord.y = t1;
	g_locked_vertices[base + 3].col.x = g_color_r;
	g_locked_vertices[base + 3].col.y = g_color_g;
	g_locked_vertices[base + 3].col.z = g_color_b;
	g_locked_vertices[base + 3].col.w = g_color_a * g_opacity;
	
	g_vertex_index++;
}

void kore_g2_draw_string(const char *utf8_text, float x, float y) {
	if (!g_font) return;
	
	const char *ptr = utf8_text;
	float xpos = x;
	float ypos = y + g_font->baseline;
	
	while (*ptr) {
		uint32_t codepoint = utf8_decode(&ptr);
		
		kore_g2_font_aligned_quad quad;
		if (kore_g2_font_get_baked_quad(g_font, codepoint, &xpos, &ypos, &quad)) {
			add_text_quad(quad.x0, quad.y0, quad.x1, quad.y1, quad.s0, quad.t0, quad.s1, quad.t1);
		}
	}
}

float kore_g2_string_width(const char *utf8_text) {
	if (!g_font) return 0.0f;
	return kore_g2_font_string_width(g_font, utf8_text);
}

void kore_g2_translate(float tx, float ty) {
	kore_matrix4x4 translation = kore_matrix4x4_translation(tx, ty, 0.0f);
	g_transform = kore_matrix4x4_multiply(&translation, &g_transform);
}

void kore_g2_rotate(float angle, float cx, float cy) {
	kore_matrix4x4 rotation = kore_matrix4x4_rotation_z(angle);
	kore_matrix4x4 t1 = kore_matrix4x4_translation(-cx, -cy, 0.0f);
	kore_matrix4x4 t2 = kore_matrix4x4_translation(cx, cy, 0.0f);
	kore_matrix4x4 temp = kore_matrix4x4_multiply(&t1, &rotation);
	g_transform = kore_matrix4x4_multiply(&temp, &g_transform);
	(void)t2;
}

void kore_g2_scale(float sx, float sy) {
	kore_matrix4x4 scale = kore_matrix4x4_scale(sx, sy, 1.0f);
	g_transform = kore_matrix4x4_multiply(&scale, &g_transform);
}

void kore_g2_push_transformation(void) {
	if (g_transform_index < 31) {
		g_transform_stack[++g_transform_index] = g_transform;
	}
}

void kore_g2_pop_transformation(void) {
	if (g_transform_index > 0) {
		g_transform = g_transform_stack[g_transform_index--];
	}
}

void kore_g2_set_opacity(float opacity) {
	g_opacity = opacity;
}

void kore_g2_push_opacity(float opacity) {
	if (g_opacity_index < 31) {
		g_opacity_stack[++g_opacity_index] = g_opacity;
		g_opacity = opacity;
	}
}

float kore_g2_pop_opacity(void) {
	if (g_opacity_index > 0) {
		g_opacity = g_opacity_stack[g_opacity_index--];
	}
	return g_opacity;
}

void kore_g2_scissor(int x, int y, int width, int height) {
	kore_gpu_command_list_set_scissor_rect(g_command_list, x, y, width, height);
}

void kore_g2_disable_scissor(void) {
	kore_gpu_command_list_set_scissor_rect(g_command_list, 0, 0, g_screen_width, g_screen_height);
}
