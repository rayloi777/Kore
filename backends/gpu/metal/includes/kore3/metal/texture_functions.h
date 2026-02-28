#ifndef KORE_METAL_TEXTURE_FUNCTIONS_HEADER
#define KORE_METAL_TEXTURE_FUNCTIONS_HEADER

#include <kore3/gpu/texture.h>

#ifdef __cplusplus
extern "C" {
#endif

void kore_metal_texture_set_name(kore_gpu_texture *texture, const char *name);
void kore_metal_texture_destroy(kore_gpu_texture *texture);
void kore_metal_texture_view_create(kore_gpu_device *device, kore_gpu_texture *texture, kore_gpu_texture_view *view);
void kore_metal_texture_view_destroy(kore_gpu_texture_view *view);
void kore_metal_texture_upload(kore_gpu_device *device, kore_gpu_texture *texture, const void *pixels, uint32_t width, uint32_t height);

#ifdef __cplusplus
}
#endif

#endif
