#ifndef KORE_VULKAN_DESCRIPTORSET_FUNCTIONS_HEADER
#define KORE_VULKAN_DESCRIPTORSET_FUNCTIONS_HEADER

#include "buffer_structs.h"
#include "descriptorset_structs.h"
#include "device_structs.h"

#include <kore3/gpu/sampler.h>
#include <kore3/gpu/texture.h>

#ifdef __cplusplus
extern "C" {
#endif

void kore_vulkan_descriptor_set_layout_create(kore_gpu_device *device, void *bindings, size_t bindings_count, kore_vulkan_descriptor_set_layout *layout);
void kore_vulkan_descriptor_set_layout_destroy(kore_vulkan_descriptor_set_layout *layout);
void kore_vulkan_descriptor_set_create(kore_gpu_device *device, const kore_vulkan_descriptor_set_layout *layout, kore_vulkan_descriptor_set *set);
void kore_vulkan_descriptor_set_destroy(kore_vulkan_descriptor_set *set);
void kore_vulkan_descriptor_set_set_buffer(kore_vulkan_descriptor_set *set, uint32_t slot, kore_gpu_buffer *buffer);
void kore_vulkan_descriptor_set_set_texture(kore_vulkan_descriptor_set *set, uint32_t slot, kore_gpu_texture *texture);
void kore_vulkan_descriptor_set_set_sampler(kore_vulkan_descriptor_set *set, uint32_t slot, kore_gpu_sampler *sampler);

void kore_vulkan_descriptor_set_set_uniform_buffer_descriptor(kore_gpu_device *device, kore_vulkan_descriptor_set *set, kore_gpu_buffer *buffer,
                                                              uint32_t index);
void kore_vulkan_descriptor_set_set_dynamic_uniform_buffer_descriptor(kore_gpu_device *device, kore_vulkan_descriptor_set *set, kore_gpu_buffer *buffer,
                                                                      uint32_t range, uint32_t index);
void kore_vulkan_descriptor_set_set_sampled_image_descriptor(kore_gpu_device *device, kore_vulkan_descriptor_set *set,
                                                             const kore_gpu_texture_view *texture_view, uint32_t index);
void kore_vulkan_descriptor_set_set_storage_image_descriptor(kore_gpu_device *device, kore_vulkan_descriptor_set *set,
                                                             const kore_gpu_texture_view *texture_view, uint32_t index);
void kore_vulkan_descriptor_set_set_sampled_image_array_descriptor(kore_gpu_device *device, kore_vulkan_descriptor_set *set,
                                                                   const kore_gpu_texture_view *texture_view, uint32_t index);
void kore_vulkan_descriptor_set_set_sampled_cube_image_descriptor(kore_gpu_device *device, kore_vulkan_descriptor_set *set,
                                                                   const kore_gpu_texture_view *texture_view, uint32_t index);
void kore_vulkan_descriptor_set_set_sampler(kore_gpu_device *device, kore_vulkan_descriptor_set *set, kore_gpu_sampler *sampler, uint32_t index);

void kore_vulkan_descriptor_set_prepare_buffer(kore_gpu_command_list *list, kore_gpu_buffer *buffer, uint64_t offset, uint64_t size);
void kore_vulkan_descriptor_set_prepare_texture(kore_gpu_command_list *list, const kore_gpu_texture_view *texture_view, bool writable);

void kore_vulkan_desciptor_set_use_free_allocation(kore_vulkan_descriptor_set *set);

#ifdef __cplusplus
}
#endif

#endif
