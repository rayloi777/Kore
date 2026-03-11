#ifndef KORE_G2_FONT_HEADER
#define KORE_G2_FONT_HEADER

#include <kore3/global.h>
#include <kore3/gpu/texture.h>
#include <kore3/gpu/commandlist.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KORE_G2_FONT_MAX_CHARS        8192
#define KORE_G2_FONT_TEXTURE_SIZE     4096
#define KORE_G2_FONT_MAX_PAGES        8
#define KORE_G2_FONT_CACHE_SIZE       256
#define KORE_G2_FONT_PENDING_BATCH    32

typedef struct kore_g2_font kore_g2_font;

typedef struct kore_g2_font_glyph_cache_entry {
    int codepoint;
    int x, y;
    int width, height;
    float xadvance;
    float xoff, yoff;
    bool baked;
    uint32_t last_access;
} kore_g2_font_glyph_cache_entry;

typedef struct kore_g2_font_glyph_page {
    kore_gpu_texture texture;
    kore_gpu_texture_view texture_view;
    int width;
    int height;
    int font_size;

    kore_g2_font_glyph_cache_entry *entries;
    int cache_capacity;
    int cache_count;
    uint32_t access_counter;

    int next_x;
    int next_y;
    int row_height;
    int max_row_height;
} kore_g2_font_glyph_page;

typedef struct kore_g2_font_glyph_pending {
    int codepoint;
    uint8_t *pixels;
    int width;
    int height;
    int tex_x;
    int tex_y;
} kore_g2_font_glyph_pending;

struct kore_g2_font {
    uint8_t *font_data;
    int font_data_size;
    float font_size;

    kore_gpu_texture texture;
    int texture_width;
    int texture_height;

    void *baked_chars;
    int *char_blocks;
    int char_blocks_count;
    int total_glyphs;

    float baseline;
    float height;

    bool dynamic_mode;
    bool deferred_mode;
    bool rebuild_needed;
    int current_page_index;

    kore_g2_font_glyph_page pages[KORE_G2_FONT_MAX_PAGES];
    int page_count;

    kore_g2_font_glyph_pending pending_glyphs[KORE_G2_FONT_PENDING_BATCH];
    int pending_count;

    int cache_hits;
    int cache_misses;
    int cache_evictions;
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

KORE_FUNC void kore_g2_font_enable_dynamic(kore_g2_font *font, bool enable);

KORE_FUNC void kore_g2_font_set_size(kore_g2_font *font, float size);
KORE_FUNC float kore_g2_font_get_size(kore_g2_font *font);

KORE_FUNC void kore_g2_font_warmup(kore_g2_font *font, const int *codepoints, int count);

KORE_FUNC void kore_g2_font_get_stats(kore_g2_font *font, int *total_requests, int *cache_hits, int *cache_misses, int *evictions);

KORE_FUNC void kore_g2_font_flush(kore_g2_font *font);

KORE_FUNC void kore_g2_font_flush_with_command_list(kore_g2_font *font, struct kore_gpu_device *device, kore_gpu_command_list *list);

KORE_FUNC bool kore_g2_font_ensure_codepoint(kore_g2_font *font, struct kore_gpu_device *device, int codepoint);

KORE_FUNC void kore_g2_font_evict_lru_page(kore_g2_font *font, struct kore_gpu_device *device);

KORE_FUNC void kore_g2_font_add_glyph(kore_g2_font *font, int codepoint);

KORE_FUNC void kore_g2_font_add_glyphs(kore_g2_font *font, const int *codepoints, int count);

KORE_FUNC void kore_g2_font_rebuild(kore_g2_font *font, struct kore_gpu_device *device);

KORE_FUNC void kore_g2_font_set_deferred(kore_g2_font *font, bool deferred);

#ifdef __cplusplus
}
#endif

#endif
