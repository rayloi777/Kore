#ifndef KORE_G2_FONT_HEADER
#define KORE_G2_FONT_HEADER

#include <kore3/global.h>
#include <kore3/gpu/texture.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KORE_G2_FONT_MAX_CHARS        8192
#define KORE_G2_FONT_TEXTURE_SIZE     4096

typedef struct kore_g2_font kore_g2_font;

struct kore_g2_font {
	uint8_t *font_data;
	int      font_data_size;
	float    font_size;

	kore_gpu_texture texture;
	int              texture_width;
	int              texture_height;

	void    *baked_chars;
	int     *char_blocks;
	int      char_blocks_count;
	int      total_glyphs;

	float baseline;
	float height;
};

typedef struct kore_g2_font_aligned_quad {
	float x0, y0, s0, t0;
	float x1, y1, s1, t1;
	float xadvance;
} kore_g2_font_aligned_quad;

KORE_FUNC bool kore_g2_font_init(kore_g2_font *font, struct kore_gpu_device *device, const char *filename, float font_size);

KORE_FUNC bool kore_g2_font_init_with_glyphs(kore_g2_font *font, struct kore_gpu_device *device, const char *filename, float font_size,
                                             const int *codepoints, int codepoints_count);

KORE_FUNC bool kore_g2_font_init_from_memory(kore_g2_font *font, struct kore_gpu_device *device, uint8_t *data, int data_size, float font_size,
                                             const int *codepoints, int codepoints_count);

KORE_FUNC void kore_g2_font_destroy(kore_g2_font *font);

KORE_FUNC bool kore_g2_font_get_baked_quad(kore_g2_font *font, int codepoint, float *xpos, float *ypos, kore_g2_font_aligned_quad *quad);

KORE_FUNC kore_gpu_texture *kore_g2_font_get_texture(kore_g2_font *font);

KORE_FUNC float kore_g2_font_string_width(kore_g2_font *font, const char *utf8_text);
KORE_FUNC float kore_g2_font_height(kore_g2_font *font);
KORE_FUNC float kore_g2_font_baseline(kore_g2_font *font);

KORE_FUNC const int *kore_g2_font_get_default_glyphs(int *count);
KORE_FUNC const int *kore_g2_font_get_traditional_chinese_glyphs(int *count);

#ifdef __cplusplus
}
#endif

#endif
