#include <kore3/vulkan/texture_functions.h>

#include "vulkanunit.h"

#include <kore3/gpu/texture.h>
#include <kore3/gpu/device.h>
#include <kore3/gpu/buffer.h>
#include <kore3/gpu/commandlist.h>
#include <string.h>

void kore_vulkan_texture_set_name(kore_gpu_texture *texture, const char *name) {
	const VkDebugUtilsObjectNameInfoEXT name_info = {
	    .sType        = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
	    .objectType   = VK_OBJECT_TYPE_IMAGE,
	    .objectHandle = (uint64_t)texture->vulkan.image,
	    .pObjectName  = name,
	};
	vulkanSetDebugUtilsObjectName(texture->vulkan.device, &name_info);
}

void kore_vulkan_texture_destroy(kore_gpu_texture *texture) {
	vkDestroyImage(texture->vulkan.device, texture->vulkan.image, NULL);
	vkFreeMemory(texture->vulkan.device, texture->vulkan.device_memory, NULL);
}

void kore_vulkan_texture_view_create(kore_gpu_device *device, kore_gpu_texture *texture, kore_gpu_texture_view *view) {
	view->texture = texture;
	view->format = texture->format;
	view->dimension = KORE_GPU_TEXTURE_VIEW_DIMENSION_2D;
	view->aspect = KORE_GPU_IMAGE_COPY_ASPECT_ALL;
	view->base_mip_level = 0;
	view->mip_level_count = 1;
	view->base_array_layer = 0;
	view->array_layer_count = 1;
}

void kore_vulkan_texture_view_destroy(kore_gpu_texture_view *view) {
}

void kore_vulkan_texture_upload(kore_gpu_device *device, kore_gpu_texture *texture, const void *pixels, uint32_t width, uint32_t height) {
	VkDevice vk_device = device->vulkan.device;
	
	VkBuffer staging_buffer;
	VkDeviceMemory staging_memory;
	
	VkBufferCreateInfo buffer_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = width * height * 4,
		.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};
	vkCreateBuffer(vk_device, &buffer_info, NULL, &staging_buffer);
	
	VkMemoryRequirements mem_reqs;
	vkGetBufferMemoryRequirements(vk_device, staging_buffer, &mem_reqs);
	
	VkMemoryAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = mem_reqs.size,
		.memoryTypeIndex = 0,
	};
	
	uint32_t memory_type_bits = mem_reqs.memoryTypeBits;
	for (uint32_t i = 0; i < 32; i++) {
		if (memory_type_bits & 1) {
			VkPhysicalDeviceMemoryProperties mem_props;
			vkGetPhysicalDeviceMemoryProperties(device->vulkan.physical_device, &mem_props);
			if (mem_props.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
				alloc_info.memoryTypeIndex = i;
				break;
			}
		}
		memory_type_bits >>= 1;
	}
	
	vkAllocateMemory(vk_device, &alloc_info, NULL, &staging_memory);
	vkBindBufferMemory(vk_device, staging_buffer, staging_memory, 0);
	
	void *data;
	vkMapMemory(vk_device, staging_memory, 0, width * height * 4, 0, &data);
	memcpy(data, pixels, width * height * 4);
	vkUnmapMemory(vk_device, staging_memory);
	
	VkCommandBuffer command_buffer;
	VkCommandBufferAllocateInfo alloc_info2 = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = device->vulkan.command_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};
	vkAllocateCommandBuffers(vk_device, &alloc_info2, &command_buffer);
	
	VkCommandBufferBeginInfo begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};
	vkBeginCommandBuffer(command_buffer, &begin_info);
	
	VkImageMemoryBarrier barrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		.image = texture->vulkan.image,
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
	};
	vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);
	
	VkBufferImageCopy region = {
		.bufferOffset = 0,
		.bufferRowLength = width,
		.bufferImageHeight = height,
		.imageSubresource = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
		.imageOffset = {0, 0, 0},
		.imageExtent = {width, height, 1},
	};
	vkCmdCopyBufferToImage(command_buffer, staging_buffer, texture->vulkan.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	
	VkImageMemoryBarrier barrier2 = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		.image = texture->vulkan.image,
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
	};
	vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier2);
	
	vkEndCommandBuffer(command_buffer);
	
	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &command_buffer,
	};
	vkQueueSubmit(device->vulkan.queue, 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(device->vulkan.queue);
	
	vkFreeCommandBuffers(vk_device, device->vulkan.command_pool, 1, &command_buffer);
	vkDestroyBuffer(vk_device, staging_buffer, NULL);
	vkFreeMemory(vk_device, staging_memory, NULL);
}

static uint32_t kore_vulkan_texture_image_layout_index(kore_gpu_texture *texture, uint32_t mip_level, uint32_t array_layer) {
	return mip_level + (array_layer * texture->vulkan.mip_level_count);
}

static VkAccessFlags get_access_mask(VkImageLayout layout) {
	switch (layout) {
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		return VK_ACCESS_TRANSFER_READ_BIT;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		VK_ACCESS_TRANSFER_WRITE_BIT;
	case VK_IMAGE_LAYOUT_GENERAL:
		return VK_ACCESS_MEMORY_READ_BIT;
	default:
		return 0;
	}
}

void kore_vulkan_texture_transition(kore_gpu_command_list *list, kore_gpu_texture *texture, VkImageLayout layout, uint32_t base_array_layer,
                                    uint32_t array_layer_count, uint32_t base_mip_level, uint32_t mip_level_count) {
	for (uint32_t array_layer = base_array_layer; array_layer < base_array_layer + array_layer_count; ++array_layer) {
		for (uint32_t mip_level = base_mip_level; mip_level < base_mip_level + mip_level_count; ++mip_level) {
			if (texture->vulkan.image_layouts[kore_vulkan_texture_image_layout_index(texture, mip_level, array_layer)] != layout) {
				VkImageMemoryBarrier barrier = {
				    .sType         = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				    .pNext         = NULL,
				    .srcAccessMask = get_access_mask(texture->vulkan.image_layouts[kore_vulkan_texture_image_layout_index(texture, mip_level, array_layer)]),
				    .dstAccessMask = get_access_mask(layout),
				    .oldLayout     = texture->vulkan.image_layouts[kore_vulkan_texture_image_layout_index(texture, mip_level, array_layer)],
				    .newLayout     = layout,
				    .image         = texture->vulkan.image,
				    .subresourceRange =
				        {
				            .aspectMask     = kore_gpu_texture_format_is_depth(texture->format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT,
				            .baseMipLevel   = mip_level,
				            .levelCount     = 1,
				            .baseArrayLayer = array_layer,
				            .layerCount     = 1,
				        },
				};

				vkCmdPipelineBarrier(list->vulkan.command_buffers[list->vulkan.active_command_buffer], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				                     VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

				texture->vulkan.image_layouts[kore_vulkan_texture_image_layout_index(texture, mip_level, array_layer)] = layout;
			}
		}
	}
}
