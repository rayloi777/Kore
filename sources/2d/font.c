#define STB_TRUETYPE_IMPLEMENTATION
#include <kore3/libs/stb_truetype.h>

#include <kore3/g2/font.h>
#include <kore3/gpu/device.h>
#include <kore3/gpu/texture.h>
#include <kore3/io/filereader.h>
#include <stdlib.h>
#include <string.h>

static const int default_glyphs[] = {
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64,
	65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96,
	97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126
};

static const int traditional_chinese_glyphs[] = {
#include "chinese_glyphs.inc"
};

const int *kore_g2_font_get_default_glyphs(int *count) {
	*count = sizeof(default_glyphs) / sizeof(default_glyphs[0]);
	return default_glyphs;
}

const int *kore_g2_font_get_traditional_chinese_glyphs(int *count) {
	*count = sizeof(traditional_chinese_glyphs) / sizeof(traditional_chinese_glyphs[0]);
	return traditional_chinese_glyphs;
}

static int find_char_index(kore_g2_font *font, int char_code) {
	int *blocks = font->char_blocks;
	int offset = 0;
	for (int i = 0; i < font->char_blocks_count; i += 2) {
		int start = blocks[i];
		int end = blocks[i + 1];
		if (char_code >= start && char_code <= end) {
			return offset + char_code - start;
		}
		offset += end - start + 1;
	}
	return 0;
}

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

static bool bake_font(kore_g2_font *font, kore_gpu_device *device, const int *codepoints, int codepoints_count) {
	stbtt_fontinfo info;
	int offset = stbtt_GetFontOffsetForIndex(font->font_data, 0);
	if (!stbtt_InitFont(&info, font->font_data, offset)) {
		return false;
	}
	
	float scale = stbtt_ScaleForPixelHeight(&info, font->font_size);
	
	int ascent, descent, line_gap;
	stbtt_GetFontVMetrics(&info, &ascent, &descent, &line_gap);
	font->baseline = (float)ascent * scale;
	font->height = (float)(ascent - descent) * scale;
	
	int *sorted_codepoints = malloc(codepoints_count * sizeof(int));
	memcpy(sorted_codepoints, codepoints, codepoints_count * sizeof(int));
	
	for (int i = 0; i < codepoints_count - 1; i++) {
		for (int j = i + 1; j < codepoints_count; j++) {
			if (sorted_codepoints[i] > sorted_codepoints[j]) {
				int temp = sorted_codepoints[i];
				sorted_codepoints[i] = sorted_codepoints[j];
				sorted_codepoints[j] = temp;
			}
		}
	}
	
	int blocks_count = 0;
	int *temp_blocks = malloc(codepoints_count * 2 * sizeof(int));
	
	int range_start = sorted_codepoints[0];
	int range_end = sorted_codepoints[0];
	
	for (int i = 1; i < codepoints_count; i++) {
		if (sorted_codepoints[i] == range_end + 1) {
			range_end = sorted_codepoints[i];
		}
		else {
			temp_blocks[blocks_count * 2] = range_start;
			temp_blocks[blocks_count * 2 + 1] = range_end;
			blocks_count++;
			range_start = sorted_codepoints[i];
			range_end = sorted_codepoints[i];
		}
	}
	temp_blocks[blocks_count * 2] = range_start;
	temp_blocks[blocks_count * 2 + 1] = range_end;
	blocks_count++;
	
	font->char_blocks = malloc(blocks_count * 2 * sizeof(int));
	memcpy(font->char_blocks, temp_blocks, blocks_count * 2 * sizeof(int));
	font->char_blocks_count = blocks_count * 2;
	free(temp_blocks);
	
	int total_glyphs = 0;
	for (int i = 0; i < blocks_count; i++) {
		total_glyphs += temp_blocks[i * 2 + 1] - temp_blocks[i * 2] + 1;
	}
	font->total_glyphs = total_glyphs;
	
	font->baked_chars = malloc(total_glyphs * sizeof(stbtt_bakedchar));
	
	int width = KORE_G2_FONT_TEXTURE_SIZE;
	int height = KORE_G2_FONT_TEXTURE_SIZE;
	uint8_t *pixels = malloc(width * height);
	
	int result = stbtt_BakeFontBitmap(font->font_data, offset, font->font_size, pixels, width, height, sorted_codepoints[0], total_glyphs, font->baked_chars);
	
	if (result <= 0) {
		free(pixels);
		free(sorted_codepoints);
		return false;
	}
	
	font->texture_width = width;
	font->texture_height = height;
	
	kore_gpu_texture_parameters tex_params = {
		.width = width,
		.height = height,
		.depth = 1,
		.array_layers = 1,
		.mip_level_count = 1,
		.sample_count = 1,
		.usage = KORE_GPU_TEXTURE_USAGE_COPY_DST | KORE_GPU_TEXTURE_USAGE_SAMPLED,
		.dimension = KORE_GPU_TEXTURE_DIMENSION_2D,
		.format = KORE_GPU_TEXTURE_FORMAT_R8_UNORM,
	};
	
	kore_gpu_device_create_texture(device, &tex_params, &font->texture);
	kore_gpu_texture_upload(device, &font->texture, pixels, width, height);
	
	free(pixels);
	free(sorted_codepoints);
	
	return true;
}

bool kore_g2_font_init(kore_g2_font *font, kore_gpu_device *device, const char *filename, float font_size) {
	int glyph_count;
	const int *glyphs = kore_g2_font_get_default_glyphs(&glyph_count);
	return kore_g2_font_init_with_glyphs(font, device, filename, font_size, glyphs, glyph_count);
}

bool kore_g2_font_init_with_glyphs(kore_g2_font *font, kore_gpu_device *device, const char *filename, float font_size,
                                   const int *codepoints, int codepoints_count) {
	kore_file_reader file;
	if (!kore_file_reader_open(&file, filename)) {
		return false;
	}
	
	int size = kore_file_reader_size(&file);
	uint8_t *data = malloc(size);
	kore_file_reader_read(&file, data, size);
	kore_file_reader_close(&file);
	
	bool result = kore_g2_font_init_from_memory(font, device, data, size, font_size, codepoints, codepoints_count);
	
	return result;
}

bool kore_g2_font_init_from_memory(kore_g2_font *font, kore_gpu_device *device, uint8_t *data, int data_size, float font_size,
                                   const int *codepoints, int codepoints_count) {
	memset(font, 0, sizeof(kore_g2_font));
	font->font_data = data;
	font->font_data_size = data_size;
	font->font_size = font_size;
	
	return bake_font(font, device, codepoints, codepoints_count);
}

void kore_g2_font_destroy(kore_g2_font *font) {
	if (font->font_data) {
		free(font->font_data);
		font->font_data = NULL;
	}
	if (font->baked_chars) {
		free(font->baked_chars);
		font->baked_chars = NULL;
	}
	if (font->char_blocks) {
		free(font->char_blocks);
		font->char_blocks = NULL;
	}
	kore_gpu_texture_destroy(&font->texture);
}

bool kore_g2_font_get_baked_quad(kore_g2_font *font, int codepoint, float *xpos, float *ypos, kore_g2_font_aligned_quad *quad) {
	stbtt_bakedchar *baked_chars = (stbtt_bakedchar *)font->baked_chars;
	
	int char_index = find_char_index(font, codepoint);
	if (char_index < 0 || char_index >= font->total_glyphs) {
		char_index = 0;
	}
	
	stbtt_bakedchar *b = &baked_chars[char_index];
	
	float ipw = 1.0f / font->texture_width;
	float iph = 1.0f / font->texture_height;
	
	int round_x = (int)(*xpos + b->xoff);
	int round_y = (int)(*ypos + b->yoff);
	
	quad->x0 = (float)round_x;
	quad->y0 = (float)round_y;
	quad->x1 = (float)(round_x + b->x1 - b->x0);
	quad->y1 = (float)(round_y + b->y1 - b->y0);
	
	quad->s0 = b->x0 * ipw;
	quad->t0 = b->y0 * iph;
	quad->s1 = b->x1 * ipw;
	quad->t1 = b->y1 * iph;
	
	quad->xadvance = b->xadvance;
	
	*xpos += b->xadvance;
	
	return true;
}

kore_gpu_texture *kore_g2_font_get_texture(kore_g2_font *font) {
	return &font->texture;
}

float kore_g2_font_string_width(kore_g2_font *font, const char *utf8_text) {
	float width = 0;
	const char *ptr = utf8_text;
	
	while (*ptr) {
		uint32_t codepoint = utf8_decode(&ptr);
		int char_index = find_char_index(font, codepoint);
		if (char_index >= 0 && char_index < font->total_glyphs) {
			stbtt_bakedchar *baked_chars = (stbtt_bakedchar *)font->baked_chars;
			width += baked_chars[char_index].xadvance;
		}
	}
	
	return width;
}

float kore_g2_font_height(kore_g2_font *font) {
	return font->height;
}

float kore_g2_font_baseline(kore_g2_font *font) {
	return font->baseline;
}
