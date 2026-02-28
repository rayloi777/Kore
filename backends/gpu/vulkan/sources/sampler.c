#include <kore3/vulkan/sampler_functions.h>

#include "vulkanunit.h"

#include <kore3/gpu/sampler.h>
#include <kore3/gpu/device.h>

void kore_vulkan_sampler_set_name(kore_gpu_sampler *sampler, const char *name) {
	const VkDebugUtilsObjectNameInfoEXT name_info = {
	    .sType        = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
	    .objectType   = VK_OBJECT_TYPE_SAMPLER,
	    .objectHandle = (uint64_t)sampler->vulkan.sampler,
	    .pObjectName  = name,
	};
	vulkanSetDebugUtilsObjectName(sampler->vulkan.device, &name_info);
}

void kore_vulkan_sampler_destroy(kore_gpu_sampler *sampler) {
	if (sampler->vulkan.sampler != VK_NULL_HANDLE) {
		vkDestroySampler(sampler->vulkan.device, sampler->vulkan.sampler, NULL);
		sampler->vulkan.sampler = VK_NULL_HANDLE;
	}
}

void kore_vulkan_device_create_default_sampler(kore_gpu_device *device, kore_gpu_sampler *sampler) {
	VkSamplerCreateInfo info = {
		.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter               = VK_FILTER_LINEAR,
		.minFilter               = VK_FILTER_LINEAR,
		.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.mipLodBias              = 0.0f,
		.anisotropyEnable        = VK_TRUE,
		.maxAnisotropy           = 16.0f,
		.compareEnable           = VK_FALSE,
		.compareOp               = VK_COMPARE_OP_ALWAYS,
		.minLod                  = 0.0f,
		.maxLod                  = VK_LOD_CLAMP_NONE,
		.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		.unnormalizedCoordinates = VK_FALSE,
	};
	VkResult result = vkCreateSampler(device->vulkan.device, &info, NULL, &sampler->vulkan.sampler);
	(void)result;
	sampler->vulkan.device = device->vulkan.device;
}
