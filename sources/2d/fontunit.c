#define STB_TRUETYPE_IMPLEMENTATION
#include <kore3/libs/stb_truetype.h>

#include <kore3/g2/font.h>
#include <kore3/gpu/device.h>
#include <kore3/gpu/texture.h>
#include <kore3/io/filereader.h>
#include <kore3/system.h>
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

static stbtt_fontinfo g_font_info;
static bool g_font_info_initialized = false;

const int *kore_g2_font_get_default_glyphs(int *count) {
	*count = sizeof(default_glyphs) / sizeof(default_glyphs[0]);
	return default_glyphs;
}

const int *kore_g2_font_get_traditional_chinese_glyphs(int *count) {
	*count = sizeof(traditional_chinese_glyphs) / sizeof(traditional_chinese_glyphs[0]);
	return traditional_chinese_glyphs;
}

static int find_char_index(kore_g2_font *font, int char_code) {
	if (!font->char_blocks || font->char_blocks_count == 0) return 0;
	
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

static void init_glyph_page(kore_g2_font_glyph_page *page, kore_gpu_device *device, int font_size) {
	page->width = KORE_G2_FONT_TEXTURE_SIZE;
	page->height = KORE_G2_FONT_TEXTURE_SIZE;
	page->font_size = font_size;
	
	page->cache_capacity = KORE_G2_FONT_CACHE_SIZE;
	page->cache_count = 0;
	page->access_counter = 0;
	
	page->entries = calloc(page->cache_capacity, sizeof(kore_g2_font_glyph_cache_entry));
	
	page->next_x = 0;
	page->next_y = 0;
	page->row_height = 0;
	page->max_row_height = 0;
	
	kore_gpu_texture_parameters tex_params = {
		.width = page->width,
		.height = page->height,
		.depth_or_array_layers = 1,
		.mip_level_count = 1,
		.sample_count = 1,
		.usage = KORE_GPU_TEXTURE_USAGE_COPY_DST | KORE_GPU_TEXTURE_USAGE_SAMPLED,
		.dimension = KORE_GPU_TEXTURE_DIMENSION_2D,
		.format = KORE_GPU_TEXTURE_FORMAT_R8_UNORM,
	};
	
	uint8_t *pixels = calloc(1, page->width * page->height);
	kore_gpu_device_create_texture(device, &tex_params, &page->texture);
	kore_gpu_texture_upload(device, &page->texture, pixels, page->width, page->height);
	free(pixels);
	
	kore_gpu_texture_view_create(device, &page->texture, &page->texture_view);
}

static void free_glyph_page(kore_g2_font_glyph_page *page, kore_gpu_device *device) {
	(void)device;
	if (page->entries) {
		free(page->entries);
		page->entries = NULL;
	}
	kore_gpu_texture_view_destroy(&page->texture_view);
	kore_gpu_texture_destroy(&page->texture);
}

static kore_g2_font_glyph_page* get_or_create_page(kore_g2_font *font, kore_gpu_device *device, int font_size) {
	for (int i = 0; i < font->page_count; i++) {
		if (font->pages[i].font_size == font_size) {
			return &font->pages[i];
		}
	}
	
	if (font->page_count >= KORE_G2_FONT_MAX_PAGES) {
		kore_g2_font_evict_lru_page(font, device);
	}
	
	init_glyph_page(&font->pages[font->page_count], device, font_size);
	return &font->pages[font->page_count++];
}

static int find_glyph_in_page(kore_g2_font_glyph_page *page, int codepoint) {
	for (int i = 0; i < page->cache_count; i++) {
		if (page->entries[i].codepoint == codepoint) {
			return i;
		}
	}
	return -1;
}

static void evict_lru_entries(kore_g2_font_glyph_page *page, int count) {
	if (page->cache_count <= 0 || count <= 0) return;
	
	qsort(page->entries, page->cache_count, sizeof(kore_g2_font_glyph_cache_entry), 
		(int (*)(const void *, const void *))strcmp);
	
	int evict = count;
	if (evict > page->cache_count) evict = page->cache_count;
	
	for (int i = 0; i < evict; i++) {
		page->entries[i].baked = false;
	}
	
	memmove(&page->entries[0], &page->entries[evict], 
		(page->cache_count - evict) * sizeof(kore_g2_font_glyph_cache_entry));
	
	page->cache_count -= evict;
}

static bool allocate_glyph_slot(kore_g2_font_glyph_page *page, int width, int height, int *out_x, int *out_y) {
	if (page->next_x + width > page->width) {
		page->next_x = 0;
		page->next_y += page->max_row_height + 1;
		page->max_row_height = 0;
	}
	
	if (page->next_y + height > page->height) {
		evict_lru_entries(page, 32);
		page->next_y += page->max_row_height + 1;
		page->next_x = 0;
		page->max_row_height = 0;
		
		if (page->next_y + height > page->height) {
			return false;
		}
	}
	
	*out_x = page->next_x;
	*out_y = page->next_y;
	
	page->next_x += width + 1;
	if (height > page->max_row_height) {
		page->max_row_height = height;
	}
	
	return true;
}

void kore_g2_font_evict_lru_page(kore_g2_font *font, kore_gpu_device *device) {
	if (font->page_count <= 1) return;
	
	int oldest_idx = 0;
	uint32_t oldest_counter = font->pages[0].access_counter;
	
	for (int i = 1; i < font->page_count; i++) {
		if (font->pages[i].access_counter < oldest_counter) {
			oldest_counter = font->pages[i].access_counter;
			oldest_idx = i;
		}
	}
	
	free_glyph_page(&font->pages[oldest_idx], device);
	
	memmove(&font->pages[oldest_idx], &font->pages[oldest_idx + 1],
		(font->page_count - oldest_idx - 1) * sizeof(kore_g2_font_glyph_page));
	
	font->page_count--;
	font->cache_evictions += 32;
}

static bool bake_glyph_to_page(kore_g2_font *font, kore_g2_font_glyph_page *page, int codepoint) {
	stbtt_fontinfo info;
	int offset = stbtt_GetFontOffsetForIndex(font->font_data, 0);
	if (!stbtt_InitFont(&info, font->font_data, offset)) {
		return false;
	}
	
	float scale = stbtt_ScaleForPixelHeight(&info, (float)page->font_size);
	
	int glyph_index = stbtt_FindGlyphIndex(&info, codepoint);
	if (glyph_index == 0) {
		return false;
	}
	
	int advance, lsb;
	stbtt_GetGlyphHMetrics(&info, glyph_index, &advance, &lsb);
	
	int x0, y0, x1, y1;
	stbtt_GetGlyphBitmapBox(&info, glyph_index, scale, scale, &x0, &y0, &x1, &y1);
	
	int width = x1 - x0;
	int height = y1 - y0;
	
	if (width <= 0 || height <= 0) {
		width = 1;
		height = 1;
	}
	
	int tex_x, tex_y;
	if (!allocate_glyph_slot(page, width, height, &tex_x, &tex_y)) {
		return false;
	}
	
	uint8_t *pixels = calloc(width * height, 1);
	
	int iv_x0 = x0, iv_y0 = y0;
	if (iv_x0 < 0) iv_x0 = 0;
	if (iv_y0 < 0) iv_y0 = 0;
	
	stbtt_MakeCodepointBitmap(&info, pixels, width, height, width, scale, scale, codepoint);
	
	font->pending_glyphs[font->pending_count].codepoint = codepoint;
	font->pending_glyphs[font->pending_count].pixels = pixels;
	font->pending_glyphs[font->pending_count].width = width;
	font->pending_glyphs[font->pending_count].height = height;
	font->pending_glyphs[font->pending_count].tex_x = tex_x;
	font->pending_glyphs[font->pending_count].tex_y = tex_y;
	font->pending_count++;
	
	if (page->cache_count >= page->cache_capacity) {
		evict_lru_entries(page, 16);
	}
	
	kore_g2_font_glyph_cache_entry *entry = &page->entries[page->cache_count++];
	entry->codepoint = codepoint;
	entry->x = tex_x;
	entry->y = tex_y;
	entry->width = width;
	entry->height = height;
	entry->xadvance = (float)advance * scale;
	entry->xoff = (float)x0;
	entry->yoff = (float)y0;
	entry->baked = true;
	entry->last_access = page->access_counter++;
	
	return true;
}

void kore_g2_font_enable_dynamic(kore_g2_font *font, bool enable) {
	font->dynamic_mode = enable;
}

void kore_g2_font_set_size(kore_g2_font *font, float size) {
	font->current_page_index = -1;
	
	for (int i = 0; i < font->page_count; i++) {
		if (font->pages[i].font_size == (int)size) {
			font->current_page_index = i;
			return;
		}
	}
	
	font->current_page_index = 0;
}

float kore_g2_font_get_size(kore_g2_font *font) {
	if (font->current_page_index >= 0 && font->current_page_index < font->page_count) {
		return (float)font->pages[font->current_page_index].font_size;
	}
	return font->font_size;
}

void kore_g2_font_warmup(kore_g2_font *font, const int *codepoints, int count) {
	if (!font->dynamic_mode || font->page_count == 0) return;
	
	kore_g2_font_glyph_page *page = &font->pages[0];
	
	for (int i = 0; i < count; i++) {
		if (find_glyph_in_page(page, codepoints[i]) < 0) {
			bake_glyph_to_page(font, page, codepoints[i]);
		}
	}
	
	kore_g2_font_flush(font);
}

void kore_g2_font_get_stats(kore_g2_font *font, int *total_requests, int *hits, int *misses, int *evictions) {
	*total_requests = font->cache_hits + font->cache_misses;
	*hits = font->cache_hits;
	*misses = font->cache_misses;
	*evictions = font->cache_evictions;
}

void kore_g2_font_add_glyph(kore_g2_font *font, int codepoint) {
	if (!font || codepoint <= 0) return;
	font->rebuild_needed = true;
}

void kore_g2_font_add_glyphs(kore_g2_font *font, const int *codepoints, int count) {
	if (!font || !codepoints || count <= 0) return;
	font->rebuild_needed = true;
}

void kore_g2_font_set_deferred(kore_g2_font *font, bool deferred) {
	if (!font) return;
	font->deferred_mode = deferred;
}

void kore_g2_font_rebuild(kore_g2_font *font, kore_gpu_device *device) {
	if (!font || !device) return;
	
	kore_log(KORE_LOG_LEVEL_INFO, "Rebuilding font texture...");
	
	font->rebuild_needed = false;
	
	stbtt_fontinfo info;
	int offset = stbtt_GetFontOffsetForIndex(font->font_data, 0);
	if (!stbtt_InitFont(&info, font->font_data, offset)) {
		kore_log(KORE_LOG_LEVEL_ERROR, "Failed to init font for rebuild");
		return;
	}
	
	float scale = stbtt_ScaleForPixelHeight(&info, font->font_size);
	
	int ascent, descent, line_gap;
	stbtt_GetFontVMetrics(&info, &ascent, &descent, &line_gap);
	font->baseline = (float)ascent * scale;
	font->height = (float)(ascent - descent) * scale;
	
	int width = KORE_G2_FONT_TEXTURE_SIZE;
	int height = KORE_G2_FONT_TEXTURE_SIZE;
	uint8_t *pixels = malloc(width * height);
	memset(pixels, 0, width * height);
	
	int count = font->total_glyphs;
	if (count <= 0 || !font->baked_chars) {
		kore_log(KORE_LOG_LEVEL_WARNING, "No glyphs to rebuild");
		free(pixels);
		return;
	}
	
	stbtt_bakedchar *chars = (stbtt_bakedchar *)font->baked_chars;
	
	stbtt_bakedchar *new_chars = malloc(count * sizeof(stbtt_bakedchar));
	
	int result = stbtt_BakeFontBitmap(font->font_data, offset, font->font_size, pixels, width, height, 
		font->char_blocks[0], count, new_chars);
	
	if (result <= 0) {
		kore_log(KORE_LOG_LEVEL_ERROR, "Failed to bake font bitmap");
		free(pixels);
		free(new_chars);
		return;
	}
	
	memcpy(chars, new_chars, count * sizeof(stbtt_bakedchar));
	free(new_chars);
	
	font->texture_width = width;
	font->texture_height = height;
	
	kore_gpu_texture_destroy(&font->texture);
	
	kore_gpu_texture_parameters tex_params = {
		.width = width,
		.height = height,
		.depth_or_array_layers = 1,
		.mip_level_count = 1,
		.sample_count = 1,
		.usage = KORE_GPU_TEXTURE_USAGE_COPY_DST | KORE_GPU_TEXTURE_USAGE_SAMPLED,
		.dimension = KORE_GPU_TEXTURE_DIMENSION_2D,
		.format = KORE_GPU_TEXTURE_FORMAT_R8_UNORM,
	};
	
	kore_gpu_device_create_texture(device, &tex_params, &font->texture);
	kore_gpu_texture_upload(device, &font->texture, pixels, width, height);
	
	free(pixels);
	
	kore_log(KORE_LOG_LEVEL_INFO, "Font rebuilt successfully, %d glyphs baked", result);
}

void kore_g2_font_flush(kore_g2_font *font) {
	if (font->pending_count == 0) return;
	
	kore_log(KORE_LOG_LEVEL_INFO, "Flushing %d pending glyphs (no cmdlist - use _with_command_list)", font->pending_count);
	font->pending_count = 0;
}

void kore_g2_font_flush_with_command_list(kore_g2_font *font, kore_gpu_device *device, kore_gpu_command_list *cmdlist) {
	if (font->pending_count == 0 || cmdlist == NULL) {
		font->pending_count = 0;
		return;
	}
	
	kore_g2_font_glyph_page *page = &font->pages[0];
	
	for (int i = 0; i < font->pending_count; i++) {
		kore_g2_font_glyph_pending *pending = &font->pending_glyphs[i];
		
		kore_gpu_buffer staging_buffer;
		kore_gpu_buffer_parameters buf_params = {
			.size = pending->width * pending->height,
			.usage_flags = KORE_GPU_BUFFER_USAGE_CPU_WRITE | KORE_GPU_BUFFER_USAGE_COPY_SRC,
		};
		kore_gpu_device_create_buffer(device, &buf_params, &staging_buffer);
		
		uint8_t *buf_ptr = (uint8_t *)kore_gpu_buffer_lock_all(&staging_buffer);
		memcpy(buf_ptr, pending->pixels, pending->width * pending->height);
		kore_gpu_buffer_unlock_all(&staging_buffer);
		
		kore_gpu_image_copy_buffer src = {
			.buffer = &staging_buffer,
			.bytes_per_row = pending->width,
			.rows_per_image = pending->height,
		};
		
		kore_gpu_image_copy_texture dst = {
			.texture = &page->texture,
			.mip_level = 0,
			.origin_x = pending->tex_x,
			.origin_y = pending->tex_y,
		};
		
		kore_gpu_command_list_copy_buffer_to_texture(cmdlist, &src, &dst, pending->width, pending->height, 1);
		
		kore_gpu_buffer_destroy(&staging_buffer);
		
		free(pending->pixels);
		pending->pixels = NULL;
	}
	
	kore_log(KORE_LOG_LEVEL_INFO, "Flushed %d glyphs to GPU", font->pending_count);
	font->pending_count = 0;
}

bool kore_g2_font_ensure_codepoint(kore_g2_font *font, kore_gpu_device *device, int codepoint) {
	if (!font->dynamic_mode || codepoint == 0) return false;
	
	kore_g2_font_glyph_page *page = &font->pages[0];
	
	int idx = find_glyph_in_page(page, codepoint);
	if (idx >= 0) {
		font->cache_hits++;
		page->entries[idx].last_access = page->access_counter++;
		return true;
	}
	
	font->cache_misses++;
	
	return bake_glyph_to_page(font, page, codepoint);
}

static bool bake_font(kore_g2_font *font, kore_gpu_device *device, const int *codepoints, int codepoints_count) {
	stbtt_fontinfo info;
	int offset = stbtt_GetFontOffsetForIndex(font->font_data, 0);
	if (!stbtt_InitFont(&info, font->font_data, offset)) {
		return false;
	}
	
	g_font_info = info;
	g_font_info_initialized = true;
	
	float scale = stbtt_ScaleForPixelHeight(&info, font->font_size);
	
	int ascent, descent, line_gap;
	stbtt_GetFontVMetrics(&info, &ascent, &descent, &line_gap);
	font->baseline = (float)ascent * scale;
	font->height = (float)(ascent - descent) * scale;
	
	font->page_count = 0;
	font->current_page_index = 0;
	font->pending_count = 0;
	font->cache_hits = 0;
	font->cache_misses = 0;
	font->cache_evictions = 0;
	font->dynamic_mode = false;
	
	if (codepoints != NULL && codepoints_count > 0) {
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
		
		int total_glyphs = 0;
		for (int i = 0; i < blocks_count; i++) {
			total_glyphs += font->char_blocks[i * 2 + 1] - font->char_blocks[i * 2] + 1;
		}
		font->total_glyphs = total_glyphs;
		
		free(temp_blocks);
		free(sorted_codepoints);
		
		font->baked_chars = malloc(font->total_glyphs * sizeof(stbtt_bakedchar));
		
		int width = KORE_G2_FONT_TEXTURE_SIZE;
		int height = KORE_G2_FONT_TEXTURE_SIZE;
		uint8_t *pixels = malloc(width * height);
		
		kore_log(KORE_LOG_LEVEL_INFO, "Baking %d glyphs, font size: %f", font->total_glyphs, font->font_size);
		
		int result = stbtt_BakeFontBitmap(font->font_data, offset, font->font_size, pixels, width, height, 
			codepoints[0], font->total_glyphs, font->baked_chars);
		
		kore_log(KORE_LOG_LEVEL_INFO, "stbtt_BakeFontBitmap result: %d pixels baked", result);
		
		font->texture_width = width;
		font->texture_height = height;
		
		kore_gpu_texture_parameters tex_params = {
			.width = width,
			.height = height,
			.depth_or_array_layers = 1,
			.mip_level_count = 1,
			.sample_count = 1,
			.usage = KORE_GPU_TEXTURE_USAGE_COPY_DST | KORE_GPU_TEXTURE_USAGE_SAMPLED,
			.dimension = KORE_GPU_TEXTURE_DIMENSION_2D,
			.format = KORE_GPU_TEXTURE_FORMAT_R8_UNORM,
		};
		
		kore_gpu_device_create_texture(device, &tex_params, &font->texture);
		kore_gpu_texture_upload(device, &font->texture, pixels, width, height);
		free(pixels);
	}
	
	init_glyph_page(&font->pages[0], device, (int)font->font_size);
	font->page_count = 1;
	
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
	kore_log(KORE_LOG_LEVEL_INFO, "Attempting to load font: %s", filename);
	if (!kore_file_reader_open(&file, filename, KORE_FILE_TYPE_ASSET)) {
		kore_log(KORE_LOG_LEVEL_ERROR, "Failed to open font file: %s", filename);
		return false;
	}
	
	int size = kore_file_reader_size(&file);
	kore_log(KORE_LOG_LEVEL_INFO, "Font file opened, size: %d", size);
	
	uint8_t *data = malloc(size);
	kore_file_reader_read(&file, data, size);
	kore_file_reader_close(&file);
	
	bool result = kore_g2_font_init_from_memory(font, device, data, size, font_size, codepoints, codepoints_count);
	
	if (!result) {
		kore_log(KORE_LOG_LEVEL_ERROR, "Font init from memory failed");
	}
	
	return result;
}

bool kore_g2_font_init_from_memory(kore_g2_font *font, kore_gpu_device *device, uint8_t *data, int data_size, float font_size,
                                   const int *codepoints, int codepoints_count) {
	memset(font, 0, sizeof(kore_g2_font));
	font->font_data = data;
	font->font_data_size = data_size;
	font->font_size = font_size;
	font->current_page_index = 0;
	
	return bake_font(font, device, codepoints, codepoints_count);
}

void kore_g2_font_destroy(kore_g2_font *font) {
	for (int i = 0; i < font->page_count; i++) {
		if (font->pages[i].entries) {
			free(font->pages[i].entries);
			font->pages[i].entries = NULL;
		}
		kore_gpu_texture_view_destroy(&font->pages[i].texture_view);
		kore_gpu_texture_destroy(&font->pages[i].texture);
	}
	
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
	if (font->texture.width > 0) {
		kore_gpu_texture_destroy(&font->texture);
	}
}

bool kore_g2_font_get_baked_quad(kore_g2_font *font, int codepoint, float *xpos, float *ypos, kore_g2_font_aligned_quad *quad) {
	if (codepoint == 0) {
		*xpos += font->font_size * 0.3f;
		return false;
	}
	
	if (font->dynamic_mode) {
		kore_g2_font_glyph_page *page = &font->pages[0];
		int idx = find_glyph_in_page(page, codepoint);
		
		if (idx < 0) {
			return false;
		}
		
		kore_g2_font_glyph_cache_entry *entry = &page->entries[idx];
		entry->last_access = page->access_counter++;
		
		float ipw = 1.0f / page->width;
		float iph = 1.0f / page->height;
		
		int round_x = (int)(*xpos + entry->xoff);
		int round_y = (int)(*ypos + entry->yoff);
		
		quad->x0 = (float)round_x;
		quad->y0 = (float)round_y;
		quad->x1 = (float)(round_x + entry->width);
		quad->y1 = (float)(round_y + entry->height);
		
		quad->s0 = (float)entry->x * ipw;
		quad->t0 = (float)entry->y * iph;
		quad->s1 = (float)(entry->x + entry->width) * ipw;
		quad->t1 = (float)(entry->y + entry->height) * iph;
		
		quad->xadvance = entry->xadvance;
		
		*xpos += entry->xadvance;
		
		return true;
	}
	
	if (!font->baked_chars) return false;
	
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
	if (font->dynamic_mode && font->page_count > 0) {
		if (font->texture.width > 0 && font->texture.height > 0 && font->pending_count == 0) {
			return &font->texture;
		}
		return &font->pages[font->current_page_index].texture;
	}
	return &font->texture;
}

float kore_g2_font_string_width(kore_g2_font *font, const char *utf8_text) {
	float width = 0;
	const char *ptr = utf8_text;
	
	while (*ptr) {
		uint32_t codepoint = utf8_decode(&ptr);
		
		if (font->dynamic_mode) {
			kore_g2_font_glyph_page *page = &font->pages[0];
			int idx = find_glyph_in_page(page, codepoint);
			if (idx >= 0) {
				width += page->entries[idx].xadvance;
			}
		}
		else if (font->baked_chars) {
			int char_index = find_char_index(font, codepoint);
			if (char_index >= 0 && char_index < font->total_glyphs) {
				stbtt_bakedchar *baked_chars = (stbtt_bakedchar *)font->baked_chars;
				width += baked_chars[char_index].xadvance;
			}
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
