#define STB_TRUETYPE_IMPLEMENTATION
#include <kore3/text.h>
#include <kore3/libs/stb_truetype.h>
#include <kore3/gpu/device.h>
#include <kore3/gpu/texture.h>
#include <kore3/gpu/buffer.h>
#include <kore3/gpu/sampler.h>
#include <kore3/io/filereader.h>
#include <kore3/math/matrix.h>

#include <stdlib.h>
#include <string.h>

#define TEXT_ATLAS_SIZE 512

typedef struct {
    int codepoint;
    int x, y;
    int width, height;
    float xoffset, yoffset;
    float xadvance;
    float s0, t0, s1, t1;
} glyph_info;

struct kore_text_font {
    stbtt_fontinfo font_info;
    uint8_t *font_data;
    float scale;
    float baseline;
    
    glyph_info *glyphs;
    int glyph_count;
    
    void *device;
};

static uint32_t utf8_decode(const char *s, int *len) {
    uint32_t c = (unsigned char)s[0];
    if (c < 0x80) {
        *len = 1;
        return c;
    }
    if ((c & 0xe0) == 0xc0) {
        *len = 2;
        return ((c & 0x1f) << 6) | (s[1] & 0x3f);
    }
    if ((c & 0xf0) == 0xe0) {
        *len = 3;
        return ((c & 0x0f) << 12) | ((s[1] & 0x3f) << 6) | (s[2] & 0x3f);
    }
    if ((c & 0xf8) == 0xf0) {
        *len = 4;
        return ((c & 0x07) << 18) | ((s[1] & 0x3f) << 12) | ((s[2] & 0x3f) << 6) | (s[3] & 0x3f);
    }
    *len = 1;
    return '?';
}

static glyph_info *find_glyph(kore_text_font *font, int codepoint) {
    for (int i = 0; i < font->glyph_count; i++) {
        if (font->glyphs[i].codepoint == codepoint) {
            return &font->glyphs[i];
        }
    }
    return NULL;
}

void kore_text_init(void *device) {
    (void)device;
}

kore_text_font *kore_text_font_create(const char *ttf_path, int *glyphs, int glyph_count, float size) {
    kore_text_font *font = (kore_text_font *)malloc(sizeof(kore_text_font));
    if (!font) return NULL;
    
    memset(font, 0, sizeof(kore_text_font));
    
    kore_file_reader reader;
    if (!kore_file_reader_open(&reader, ttf_path, KORE_FILE_TYPE_ASSET)) {
        free(font);
        return NULL;
    }
    
    font->font_data = (uint8_t *)malloc(reader.size);
    kore_file_reader_read(&reader, font->font_data, reader.size);
    kore_file_reader_close(&reader);
    
    if (!stbtt_InitFont(&font->font_info, font->font_data, 0)) {
        free(font->font_data);
        free(font);
        return NULL;
    }
    
    font->scale = stbtt_ScaleForPixelHeight(&font->font_info, size);
    
    int ascent, descent, line_gap;
    stbtt_GetFontVMetrics(&font->font_info, &ascent, &descent, &line_gap);
    font->baseline = (float)(ascent - descent + line_gap) * font->scale;
    
    font->glyph_count = glyph_count;
    font->glyphs = (glyph_info *)malloc(sizeof(glyph_info) * glyph_count);
    
    uint8_t *atlas_pixels = (uint8_t *)calloc(TEXT_ATLAS_SIZE * TEXT_ATLAS_SIZE, 1);
    
    int pw = TEXT_ATLAS_SIZE;
    int ph = TEXT_ATLAS_SIZE;
    int x = 1, y = 1, max_h = 0;
    
    for (int i = 0; i < glyph_count; i++) {
        int codepoint = glyphs[i];
        
        int w, h, xoff, yoff;
        uint8_t *bitmap = stbtt_GetCodepointBitmap(&font->font_info, 0, font->scale, codepoint, &w, &h, &xoff, &yoff);
        
        if (!bitmap) {
            w = 0; h = 0; xoff = 0; yoff = 0;
        }
        
        if (x + w + 1 >= pw) {
            x = 1;
            y += max_h + 1;
            max_h = 0;
        }
        
        if (y + h >= ph) {
            if (bitmap) stbtt_FreeBitmap(bitmap, NULL);
            font->glyphs[i].width = 0;
            font->glyphs[i].height = 0;
            continue;
        }
        
        if (h > max_h) max_h = h;
        
        font->glyphs[i].codepoint = codepoint;
        font->glyphs[i].x = x;
        font->glyphs[i].y = y;
        font->glyphs[i].width = w;
        font->glyphs[i].height = h;
        font->glyphs[i].xoffset = (float)xoff;
        font->glyphs[i].yoffset = (float)yoff;
        
        int advance;
        stbtt_GetCodepointHMetrics(&font->font_info, codepoint, &advance, NULL);
        font->glyphs[i].xadvance = (float)advance * font->scale;
        
        font->glyphs[i].s0 = (float)x / (float)pw;
        font->glyphs[i].t0 = (float)y / (float)ph;
        font->glyphs[i].s1 = (float)(x + w) / (float)pw;
        font->glyphs[i].t1 = (float)(y + h) / (float)ph;
        
        if (bitmap && w > 0 && h > 0) {
            for (int py = 0; py < h; py++) {
                for (int px = 0; px < w; px++) {
                    atlas_pixels[(y + py) * pw + (x + px)] = bitmap[py * w + px];
                }
            }
            stbtt_FreeBitmap(bitmap, NULL);
        }
        
        x += w + 1;
    }
    
    free(atlas_pixels);
    
    return font;
}

void kore_text_draw(kore_text_font *font, const char *utf8_text, float x, float y, float r, float g, float b, float a) {
    (void)font;
    (void)utf8_text;
    (void)x;
    (void)y;
    (void)r;
    (void)g;
    (void)b;
    (void)a;
}

float kore_text_width(kore_text_font *font, const char *utf8_text) {
    if (!font || !utf8_text) return 0.0f;
    
    float width = 0.0f;
    const char *p = utf8_text;
    
    while (*p) {
        int len;
        uint32_t codepoint = utf8_decode(p, &len);
        p += len;
        
        glyph_info *g = find_glyph(font, codepoint);
        if (g) {
            width += g->xadvance;
        }
    }
    
    return width;
}

void kore_text_font_destroy(kore_text_font *font) {
    if (!font) return;
    
    if (font->font_data) free(font->font_data);
    if (font->glyphs) free(font->glyphs);
    free(font);
}
