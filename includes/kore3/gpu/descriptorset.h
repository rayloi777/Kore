#ifndef KORE_GPU_DESCRIPTORSET_HEADER
#define KORE_GPU_DESCRIPTORSET_HEADER

#include <kore3/global.h>

#include "api.h"
#include "buffer.h"
#include "texture.h"
#include "sampler.h"

#if defined(KORE_METAL)
#include <kore3/metal/descriptorset_structs.h>
#elif defined(KORE_VULKAN)
#include <kore3/vulkan/descriptorset_structs.h>
#elif defined(KORE_OPENGL)
#include <kore3/opengl/descriptorset_structs.h>
#elif defined(KORE_DIRECT3D11)
#include <kore3/direct3d11/descriptorset_structs.h>
#elif defined(KORE_DIRECT3D12)
#include <kore3/direct3d12/descriptorset_structs.h>
#elif defined(KORE_WEBGPU)
#include <kore3/webgpu/descriptorset_structs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum kore_gpu_descriptor_type {
	KORE_GPU_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	KORE_GPU_DESCRIPTOR_TYPE_SAMPLED_TEXTURE,
	KORE_GPU_DESCRIPTOR_TYPE_STORAGE_IMAGE,
	KORE_GPU_DESCRIPTOR_TYPE_SAMPLER,
} kore_gpu_descriptor_type;

typedef struct kore_gpu_descriptor_binding {
	kore_gpu_descriptor_type type;
	union {
		kore_gpu_buffer *buffer;
		kore_gpu_texture *texture;
		kore_gpu_sampler *sampler;
	};
	uint32_t shader_location;
} kore_gpu_descriptor_binding;

typedef struct kore_gpu_descriptor_set_layout {
	KORE_GPU_IMPL(descriptor_set_layout);
} kore_gpu_descriptor_set_layout;

typedef struct kore_gpu_descriptor_set {
	KORE_GPU_IMPL(descriptor_set);
} kore_gpu_descriptor_set;

KORE_FUNC void kore_gpu_descriptor_set_layout_create(kore_gpu_device *device, kore_gpu_descriptor_binding *bindings,
                                                      size_t bindings_count, kore_gpu_descriptor_set_layout *layout);

KORE_FUNC void kore_gpu_descriptor_set_layout_destroy(kore_gpu_descriptor_set_layout *layout);

KORE_FUNC void kore_gpu_descriptor_set_create(kore_gpu_device *device, const kore_gpu_descriptor_set_layout *layout,
                                                kore_gpu_descriptor_set *set);

KORE_FUNC void kore_gpu_descriptor_set_destroy(kore_gpu_descriptor_set *set);

KORE_FUNC void kore_gpu_descriptor_set_set_buffer(kore_gpu_descriptor_set *set, uint32_t slot, kore_gpu_buffer *buffer);

KORE_FUNC void kore_gpu_descriptor_set_set_texture(kore_gpu_descriptor_set *set, uint32_t slot, kore_gpu_texture *texture);

KORE_FUNC void kore_gpu_descriptor_set_set_sampler(kore_gpu_descriptor_set *set, uint32_t slot, kore_gpu_sampler *sampler);

#ifdef __cplusplus
}
#endif

#endif
