#include <kore3/gpu/descriptorset.h>

#if defined(KORE_METAL)
#include <kore3/metal/descriptorset_functions.h>
#include <kore3/metal/descriptorset_structs.h>
#elif defined(KORE_VULKAN)
#include <kore3/vulkan/descriptorset_functions.h>
#include <kore3/vulkan/descriptorset_structs.h>
#elif defined(KORE_OPENGL)
#include <kore3/opengl/descriptorset_functions.h>
#elif defined(KORE_DIRECT3D11)
#include <kore3/direct3d11/descriptorset_functions.h>
#elif defined(KORE_DIRECT3D12)
#include <kore3/direct3d12/descriptorset_functions.h>
#elif defined(KORE_WEBGPU)
#include <kore3/webgpu/descriptorset_functions.h>
#elif defined(KORE_KOMPJUTA)
#include <kore3/kompjuta/descriptorset_functions.h>
#endif

#include <assert.h>

void kore_gpu_descriptor_set_layout_create(kore_gpu_device *device, kore_gpu_descriptor_binding *bindings,
                                          size_t bindings_count, kore_gpu_descriptor_set_layout *layout) {
#if defined(KORE_METAL)
	kore_metal_descriptor_set_layout_create(device, bindings, bindings_count, (kore_metal_descriptor_set_layout *)layout);
#elif defined(KORE_VULKAN)
	kore_vulkan_descriptor_set_layout_create(device, bindings, bindings_count, (kore_vulkan_descriptor_set_layout *)layout);
#endif
}

void kore_gpu_descriptor_set_layout_destroy(kore_gpu_descriptor_set_layout *layout) {
#if defined(KORE_METAL)
	kore_metal_descriptor_set_layout_destroy((kore_metal_descriptor_set_layout *)layout);
#elif defined(KORE_VULKAN)
	kore_vulkan_descriptor_set_layout_destroy((kore_vulkan_descriptor_set_layout *)layout);
#endif
}

void kore_gpu_descriptor_set_create(kore_gpu_device *device, const kore_gpu_descriptor_set_layout *layout,
                                    kore_gpu_descriptor_set *set) {
#if defined(KORE_METAL)
	kore_metal_descriptor_set_create(device, (const kore_metal_descriptor_set_layout *)layout, (kore_metal_descriptor_set *)set);
#elif defined(KORE_VULKAN)
	kore_vulkan_descriptor_set_create(device, (const kore_vulkan_descriptor_set_layout *)layout, (kore_vulkan_descriptor_set *)set);
#endif
}

void kore_gpu_descriptor_set_destroy(kore_gpu_descriptor_set *set) {
#if defined(KORE_METAL)
	kore_metal_descriptor_set_destroy((kore_metal_descriptor_set *)set);
#elif defined(KORE_VULKAN)
	kore_vulkan_descriptor_set_destroy((kore_vulkan_descriptor_set *)set);
#endif
}

void kore_gpu_descriptor_set_set_buffer(kore_gpu_descriptor_set *set, uint32_t slot, kore_gpu_buffer *buffer) {
#if defined(KORE_METAL)
	kore_metal_descriptor_set_set_buffer((kore_metal_descriptor_set *)set, slot, buffer);
#elif defined(KORE_VULKAN)
	kore_vulkan_descriptor_set_set_buffer((kore_vulkan_descriptor_set *)set, slot, buffer);
#endif
}

void kore_gpu_descriptor_set_set_texture(kore_gpu_descriptor_set *set, uint32_t slot, kore_gpu_texture *texture) {
#if defined(KORE_METAL)
	kore_metal_descriptor_set_set_texture((kore_metal_descriptor_set *)set, slot, texture);
#elif defined(KORE_VULKAN)
	kore_vulkan_descriptor_set_set_texture((kore_vulkan_descriptor_set *)set, slot, texture);
#endif
}

void kore_gpu_descriptor_set_set_sampler(kore_gpu_descriptor_set *set, uint32_t slot, kore_gpu_sampler *sampler) {
#if defined(KORE_METAL)
	kore_metal_descriptor_set_set_sampler((kore_metal_descriptor_set *)set, slot, sampler);
#elif defined(KORE_VULKAN)
	kore_vulkan_descriptor_set_set_sampler((kore_vulkan_descriptor_set *)set, slot, sampler);
#endif
}
