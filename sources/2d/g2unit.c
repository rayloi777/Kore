#include <kore3/g2/g2.h>
#include <kore3/gpu/buffer.h>
#include <kore3/gpu/pipeline.h>
#include <kore3/gpu/sampler.h>
#include <kore3/math/matrix.h>

#include <kong.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef KORE_G2

#define MAX_BUFFER_SIZE 1000
#define MAX_VERTICES (MAX_BUFFER_SIZE * 4)
#define MAX_INDICES (MAX_BUFFER_SIZE * 6)

typedef struct {
    float x, y;
    float u, v;
    float r, g, b, a;
} g2_vertex;

static void g2_to_vertex(const g2_vertex *src, vertex_in *dst) {
    dst->pos.x = src->x;
    dst->pos.y = src->y;
    dst->uv.x = src->u;
    dst->uv.y = src->v;
    dst->color.x = src->r;
    dst->color.y = src->g;
    dst->color.z = src->b;
    dst->color.w = src->a;
}

static kore_gpu_device *g_device = NULL;
static int g_screen_width = 0;
static int g_screen_height = 0;
static kore_gpu_command_list *g_command_list = NULL;

static vertex_in_buffer g_vertices;
static kore_gpu_buffer g_index_buffer;
static kore_gpu_buffer g_constants;
static kore_gpu_texture *g_current_texture = NULL;
static kore_gpu_texture_view g_current_texture_view;
static kore_gpu_sampler g_sampler;
static everything_set g_descriptor_set;

static int g_vertex_count = 0;
static int g_index_count = 0;

static kore_g2_font *g_font = NULL;
static float g_font_size = 16.0f;
static float g_color_r = 1.0f, g_color_g = 1.0f, g_color_b = 1.0f, g_color_a = 1.0f;
static float g_opacity = 1.0f;

static kore_matrix3x3 g_projection;
static kore_matrix3x3 g_transform;
static kore_matrix3x3 g_transform_stack[32];
static int g_transform_index = 0;

static float g_opacity_stack[32];
static int g_opacity_index = 0;

static bool g_scissor_enabled = false;
static int g_scissor_x = 0;
static int g_scissor_y = 0;
static int g_scissor_width = 0;
static int g_scissor_height = 0;

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
	
	g_projection = kore_matrix3x3_identity();
	g_transform = kore_matrix3x3_identity();
	g_transform_index = 0;
	g_opacity = 1.0f;
	g_opacity_index = 0;
	
	// Create vertex buffer
	kong_create_buffer_vertex_in(device, MAX_VERTICES, &g_vertices);
	
	// Create index buffer
	{
		kore_gpu_buffer_parameters params = {
			.size = MAX_INDICES * sizeof(uint16_t),
			.usage_flags = KORE_GPU_BUFFER_USAGE_INDEX | KORE_GPU_BUFFER_USAGE_CPU_WRITE,
		};
		kore_gpu_device_create_buffer(device, &params, &g_index_buffer);
		
		uint16_t *indices = (uint16_t *)kore_gpu_buffer_lock_all(&g_index_buffer);
		for (int i = 0; i < MAX_BUFFER_SIZE; i++) {
			uint16_t base = i * 4;
			indices[i * 6 + 0] = base + 0;
			indices[i * 6 + 1] = base + 1;
			indices[i * 6 + 2] = base + 2;
			indices[i * 6 + 3] = base + 0;
			indices[i * 6 + 4] = base + 2;
			indices[i * 6 + 5] = base + 3;
		}
		kore_gpu_buffer_unlock(&g_index_buffer);
	}
	
	// Create constants buffer
	constants_type_buffer_create(device, &g_constants, KORE_GPU_MAX_FRAMEBUFFERS);
	
	// Create default sampler
	{
		kore_gpu_sampler_parameters params = {
			.address_mode_u = KORE_GPU_ADDRESS_MODE_CLAMP_TO_EDGE,
			.address_mode_v = KORE_GPU_ADDRESS_MODE_CLAMP_TO_EDGE,
			.mag_filter = KORE_GPU_FILTER_MODE_LINEAR,
			.min_filter = KORE_GPU_FILTER_MODE_LINEAR,
		};
		kore_gpu_device_create_sampler(device, &params, &g_sampler);
	}
	
	g_current_texture = NULL;
	g_vertex_count = 0;
	g_index_count = 0;
}

void kore_g2_begin(kore_gpu_command_list *list) {
	g_command_list = list;
	g_vertex_count = 0;
}

void kore_g2_flush(void) {
	if (g_vertex_count == 0 || g_command_list == NULL) {
		return;
	}
	
	kore_matrix3x3 mvp = kore_matrix3x3_identity();
	mvp.m[0] = 2.0f / g_screen_width;
	mvp.m[1] = 0.0f;
	mvp.m[2] = -1.0f;
	mvp.m[3] = 0.0f;
	mvp.m[4] = 2.0f / g_screen_height;
	mvp.m[5] = -1.0f;
	mvp.m[6] = 0.0f;
	mvp.m[7] = 0.0f;
	mvp.m[8] = 1.0f;
	
	constants_type *constants_data = constants_type_buffer_lock(&g_constants, 0, 1);
	constants_data->mvp = mvp;
	constants_type_buffer_unlock(&g_constants);
	
	kong_set_render_pipeline_pipeline(g_command_list);
	
	kong_set_descriptor_set_everything(g_command_list, &g_descriptor_set, 0);
	
	kong_set_vertex_buffer_vertex_in(g_command_list, &g_vertices);
	
	kore_gpu_command_list_set_index_buffer(g_command_list, &g_index_buffer, KORE_GPU_INDEX_FORMAT_UINT16, 0);
	
	kore_gpu_command_list_draw_indexed(g_command_list, g_index_count, 1, 0, 0, 0);
	
	g_vertex_count = 0;
	g_index_count = 0;
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
	if (texture == NULL) {
		return;
	}
	float w = (float)texture->width;
	float h = (float)texture->height;
	kore_g2_draw_scaled_image(texture, x, y, w, h);
}

void kore_g2_draw_scaled_image(kore_gpu_texture *texture, float dx, float dy, float dw, float dh) {
	if (texture == NULL || g_command_list == NULL) {
		return;
	}
	
	if (g_vertex_count + 4 > MAX_VERTICES) {
		kore_g2_flush();
	}
	
	if (g_current_texture != texture) {
		if (g_current_texture != NULL) {
			kore_g2_flush();
		}
		g_current_texture = texture;
		kore_gpu_texture_view_create(g_device, texture, &g_current_texture_view);
		
		everything_parameters params = {
			.constants = &g_constants,
			.tex = g_current_texture_view,
			.sam = &g_sampler,
		};
		kong_create_everything_set(g_device, &params, &g_descriptor_set);
	}
	
	vertex_in *verts = kong_vertex_in_buffer_lock(&g_vertices);
	int base = g_vertex_count;
	
	float r = g_color_r * g_opacity;
	float g_col = g_color_g * g_opacity;
	float b = g_color_b * g_opacity;
	float a = g_color_a * g_opacity;
	
	g2_vertex temp[4] = {
		{.x = dx, .y = dy, .u = 0.0f, .v = 0.0f, .r = r, .g = g_col, .b = b, .a = a},
		{.x = dx + dw, .y = dy, .u = 1.0f, .v = 0.0f, .r = r, .g = g_col, .b = b, .a = a},
		{.x = dx + dw, .y = dy + dh, .u = 1.0f, .v = 1.0f, .r = r, .g = g_col, .b = b, .a = a},
		{.x = dx, .y = dy + dh, .u = 0.0f, .v = 1.0f, .r = r, .g = g_col, .b = b, .a = a},
	};
	
	for (int i = 0; i < 4; i++) {
		g2_to_vertex(&temp[i], &verts[base + i]);
	}
	
	kong_vertex_in_buffer_unlock(&g_vertices);
	
	g_vertex_count += 4;
	g_index_count += 6;
}

void kore_g2_draw_sub_image(kore_gpu_texture *texture, float x, float y, float sx, float sy, float sw, float sh) {
	if (texture == NULL || g_command_list == NULL) {
		return;
	}
	
	float tex_w = (float)texture->width;
	float tex_h = (float)texture->height;
	
	float u0 = sx / tex_w;
	float v0 = sy / tex_h;
	float u1 = (sx + sw) / tex_w;
	float v1 = (sy + sh) / tex_h;
	
	if (g_vertex_count + 4 > MAX_VERTICES) {
		kore_g2_flush();
	}
	
	if (g_current_texture != texture) {
		if (g_current_texture != NULL) {
			kore_g2_flush();
		}
		g_current_texture = texture;
		kore_gpu_texture_view_create(g_device, texture, &g_current_texture_view);
		
		everything_parameters params = {
			.constants = &g_constants,
			.tex = g_current_texture_view,
			.sam = &g_sampler,
		};
		kong_create_everything_set(g_device, &params, &g_descriptor_set);
	}
	
	vertex_in *verts = kong_vertex_in_buffer_lock(&g_vertices);
	int base = g_vertex_count;
	
	float r = g_color_r * g_opacity;
	float g_col = g_color_g * g_opacity;
	float b = g_color_b * g_opacity;
	float a = g_color_a * g_opacity;
	
	g2_vertex temp[4] = {
		{.x = x, .y = y, .u = u0, .v = v0, .r = r, .g = g_col, .b = b, .a = a},
		{.x = x + sw, .y = y, .u = u1, .v = v0, .r = r, .g = g_col, .b = b, .a = a},
		{.x = x + sw, .y = y + sh, .u = u1, .v = v1, .r = r, .g = g_col, .b = b, .a = a},
		{.x = x, .y = y + sh, .u = u0, .v = v1, .r = r, .g = g_col, .b = b, .a = a},
	};
	
	for (int i = 0; i < 4; i++) {
		g2_to_vertex(&temp[i], &verts[base + i]);
	}
	
	kong_vertex_in_buffer_unlock(&g_vertices);
	
	g_vertex_count += 4;
	g_index_count += 6;
}

void kore_g2_draw_scaled_sub_image(kore_gpu_texture *texture, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh) {
	if (texture == NULL || g_command_list == NULL) {
		return;
	}
	
	float tex_w = (float)texture->width;
	float tex_h = (float)texture->height;
	
	float u0 = sx / tex_w;
	float v0 = sy / tex_h;
	float u1 = (sx + sw) / tex_w;
	float v1 = (sy + sh) / tex_h;
	
	if (g_vertex_count + 4 > MAX_VERTICES) {
		kore_g2_flush();
	}
	
	if (g_current_texture != texture) {
		if (g_current_texture != NULL) {
			kore_g2_flush();
		}
		g_current_texture = texture;
		kore_gpu_texture_view_create(g_device, texture, &g_current_texture_view);
		
		everything_parameters params = {
			.constants = &g_constants,
			.tex = g_current_texture_view,
			.sam = &g_sampler,
		};
		kong_create_everything_set(g_device, &params, &g_descriptor_set);
	}
	
	vertex_in *verts = kong_vertex_in_buffer_lock(&g_vertices);
	int base = g_vertex_count;
	
	float r = g_color_r * g_opacity;
	float g_col = g_color_g * g_opacity;
	float b = g_color_b * g_opacity;
	float a = g_color_a * g_opacity;
	
	g2_vertex temp[4] = {
		{.x = dx, .y = dy, .u = u0, .v = v0, .r = r, .g = g_col, .b = b, .a = a},
		{.x = dx + dw, .y = dy, .u = u1, .v = v0, .r = r, .g = g_col, .b = b, .a = a},
		{.x = dx + dw, .y = dy + dh, .u = u1, .v = v1, .r = r, .g = g_col, .b = b, .a = a},
		{.x = dx, .y = dy + dh, .u = u0, .v = v1, .r = r, .g = g_col, .b = b, .a = a},
	};
	
	for (int i = 0; i < 4; i++) {
		g2_to_vertex(&temp[i], &verts[base + i]);
	}
	
	kong_vertex_in_buffer_unlock(&g_vertices);
	
	g_vertex_count += 4;
	g_index_count += 6;
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
	if (utf8_text == NULL || g_font == NULL || g_command_list == NULL) {
		return;
	}
	
	kore_gpu_texture *font_tex = kore_g2_font_get_texture(g_font);
	if (font_tex == NULL) {
		return;
	}
	
	float xpos = x;
	float ypos = y + kore_g2_font_baseline(g_font);
	
	const char *p = utf8_text;
	while (*p) {
		uint32_t codepoint = utf8_decode(&p);
		
		kore_g2_font_aligned_quad quad;
		if (kore_g2_font_get_baked_quad(g_font, codepoint, &xpos, &ypos, &quad)) {
			printf("Draw glyph: codepoint=%d (0x%x), quad: x0=%.1f y0=%.1f x1=%.1f y1=%.1f s0=%.3f t0=%.3f s1=%.3f t1=%.3f\n", 
				codepoint, codepoint, quad.x0, quad.y0, quad.x1, quad.y1, quad.s0, quad.t0, quad.s1, quad.t1);
			if (g_vertex_count + 4 > MAX_VERTICES) {
				kore_g2_flush();
			}
			
			if (g_current_texture != font_tex) {
				if (g_current_texture != NULL) {
					kore_g2_flush();
				}
				g_current_texture = font_tex;
				kore_gpu_texture_view_create(g_device, font_tex, &g_current_texture_view);
				
				everything_parameters params = {
					.constants = &g_constants,
					.tex = g_current_texture_view,
					.sam = &g_sampler,
				};
				kong_create_everything_set(g_device, &params, &g_descriptor_set);
			}
			
			vertex_in *verts = kong_vertex_in_buffer_lock(&g_vertices);
			int base = g_vertex_count;
			
			float r = g_color_r * g_opacity;
			float g_col = g_color_g * g_opacity;
			float b = g_color_b * g_opacity;
			float a = g_color_a * g_opacity;
			
			g2_vertex temp[4] = {
				{.x = quad.x0, .y = quad.y0, .u = quad.s0, .v = 1.0f - quad.t0, .r = r, .g = g_col, .b = b, .a = a},
				{.x = quad.x1, .y = quad.y0, .u = quad.s1, .v = 1.0f - quad.t0, .r = r, .g = g_col, .b = b, .a = a},
				{.x = quad.x1, .y = quad.y1, .u = quad.s1, .v = 1.0f - quad.t1, .r = r, .g = g_col, .b = b, .a = a},
				{.x = quad.x0, .y = quad.y1, .u = quad.s0, .v = 1.0f - quad.t1, .r = r, .g = g_col, .b = b, .a = a},
			};
			
			for (int i = 0; i < 4; i++) {
				g2_to_vertex(&temp[i], &verts[base + i]);
			}
			
			kong_vertex_in_buffer_unlock(&g_vertices);
			
			g_vertex_count += 4;
			g_index_count += 6;
		}
	}
}

float kore_g2_string_width(const char *utf8_text) {
	if (utf8_text == NULL || g_font == NULL) {
		return 0.0f;
	}
	return kore_g2_font_string_width(g_font, utf8_text);
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

#endif
