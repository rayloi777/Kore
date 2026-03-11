#ifndef KORE_TEXT_HEADER
#define KORE_TEXT_HEADER

#include <kore3/global.h>

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct draw_font draw_font;

KORE_FUNC void draw_init(void *device, void *command_list);

KORE_FUNC void draw_shutdown(void);

KORE_FUNC void draw_set_viewport(int width, int height);

KORE_FUNC draw_font *draw_font_create(const char *ttf_path, int *glyphs, int glyph_count, float size);

KORE_FUNC void draw_string(draw_font *font, const char *utf8_text, float x, float y, float r, float g, float b, float a);

KORE_FUNC float draw_string_width(draw_font *font, const char *utf8_text);

KORE_FUNC void draw_font_destroy(draw_font *font);

#ifdef __cplusplus
}
#endif

#endif
