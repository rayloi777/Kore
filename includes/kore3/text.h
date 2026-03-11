#ifndef KORE_TEXT_HEADER
#define KORE_TEXT_HEADER

#include <kore3/global.h>

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kore_text_font kore_text_font;

KORE_FUNC void kore_text_init(void *device);

KORE_FUNC kore_text_font *kore_text_font_create(const char *ttf_path, int *glyphs, int glyph_count, float size);

KORE_FUNC void kore_text_draw(kore_text_font *font, const char *utf8_text, float x, float y, float r, float g, float b, float a);

KORE_FUNC float kore_text_width(kore_text_font *font, const char *utf8_text);

KORE_FUNC void kore_text_font_destroy(kore_text_font *font);

#ifdef __cplusplus
}
#endif

#endif
