#ifndef KORE_METAL_DESCRIPTORSET_FUNCTIONS_HEADER
#define KORE_METAL_DESCRIPTORSET_FUNCTIONS_HEADER

#include <kore3/metal/buffer_structs.h>
#include <kore3/metal/descriptorset_structs.h>
#include <kore3/metal/device_structs.h>

#include <kore3/gpu/sampler.h>
#include <kore3/gpu/texture.h>

#ifdef __cplusplus
extern "C" {
#endif

void kore_metal_descriptor_set_layout_create(kore_gpu_device *device, void *bindings, size_t bindings_count, kore_metal_descriptor_set_layout *layout);
void kore_metal_descriptor_set_layout_destroy(kore_metal_descriptor_set_layout *layout);
void kore_metal_descriptor_set_create(kore_gpu_device *device, const kore_metal_descriptor_set_layout *layout, kore_metal_descriptor_set *set);
void kore_metal_descriptor_set_destroy(kore_metal_descriptor_set *set);
void kore_metal_descriptor_set_set_buffer(kore_metal_descriptor_set *set, uint32_t slot, kore_gpu_buffer *buffer);
void kore_metal_descriptor_set_set_texture(kore_metal_descriptor_set *set, uint32_t slot, kore_gpu_texture *texture);
void kore_metal_descriptor_set_set_sampler(kore_metal_descriptor_set *set, uint32_t slot, kore_gpu_sampler *sampler);

void kore_metal_descriptor_set_prepare_buffer(kore_gpu_command_list *list, kore_gpu_buffer *buffer, uint32_t offset, uint32_t size);
void kore_metal_descriptor_set_prepare_texture(kore_gpu_command_list *list, void *texture_view, bool writable);

#ifdef __cplusplus
}
#endif

#endif
