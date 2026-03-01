#ifndef KORE_G2_HEADER
#define KORE_G2_HEADER

#include <kore3/global.h>
#include <kore3/gpu/commandlist.h>
#include <kore3/gpu/device.h>
#include <kore3/g2/font.h>

#ifdef __cplusplus
extern "C" {
#endif

KORE_FUNC void kore_g2_init(kore_gpu_device *device, int screen_width, int screen_height);
KORE_FUNC void kore_g2_begin(kore_gpu_command_list *list);
KORE_FUNC void kore_g2_end(void);
KORE_FUNC void kore_g2_flush(void);

KORE_FUNC void kore_g2_clear(float r, float g, float b, float a);
KORE_FUNC void kore_g2_draw_image(kore_gpu_texture *texture, float x, float y);
KORE_FUNC void kore_g2_draw_scaled_image(kore_gpu_texture *texture, float dx, float dy, float dw, float dh);
KORE_FUNC void kore_g2_draw_sub_image(kore_gpu_texture *texture, float x, float y, float sx, float sy, float sw, float sh);
KORE_FUNC void kore_g2_draw_scaled_sub_image(kore_gpu_texture *texture, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh);

KORE_FUNC void kore_g2_set_font(kore_g2_font *font);
KORE_FUNC void kore_g2_set_font_size(float size);
KORE_FUNC void kore_g2_set_color(float r, float g, float b, float a);
KORE_FUNC void kore_g2_set_color_uint(uint32_t color);
KORE_FUNC void kore_g2_draw_string(const char *utf8_text, float x, float y);
KORE_FUNC float kore_g2_string_width(const char *utf8_text);

KORE_FUNC void kore_g2_translate(float tx, float ty);
KORE_FUNC void kore_g2_rotate(float angle, float cx, float cy);
KORE_FUNC void kore_g2_scale(float sx, float sy);
KORE_FUNC void kore_g2_push_transformation(void);
KORE_FUNC void kore_g2_pop_transformation(void);

KORE_FUNC void kore_g2_set_opacity(float opacity);
KORE_FUNC void kore_g2_push_opacity(float opacity);
KORE_FUNC float kore_g2_pop_opacity(void);

KORE_FUNC void kore_g2_scissor(int x, int y, int width, int height);
KORE_FUNC void kore_g2_disable_scissor(void);

#ifdef __cplusplus
}
#endif

#endif
