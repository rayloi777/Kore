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

#define KORE_BASIC_GLYPHS_COUNT 95

static const int kore_basic_glyphs[KORE_BASIC_GLYPHS_COUNT] = {
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
    64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
    96, 97, 98, 99,
    100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115,
    116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126,
};

#ifdef __cplusplus
}
#endif

#endif
