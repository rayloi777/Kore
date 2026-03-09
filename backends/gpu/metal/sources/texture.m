#include <kore3/metal/texture_functions.h>

#include "metalunit.h"

#include <kore3/gpu/texture.h>
#include <kore3/gpu/textureformat.h>
#include <kore3/gpu/device.h>
#include <kore3/gpu/buffer.h>
#include <kore3/gpu/commandlist.h>
#include <string.h>

void kore_metal_texture_set_name(kore_gpu_texture *texture, const char *name) {
	id<MTLTexture> metal_texture = (__bridge id<MTLTexture>)texture->metal.texture;
	metal_texture.label = [NSString stringWithUTF8String:name];
}

void kore_metal_texture_destroy(kore_gpu_texture *texture) {
	if (texture->metal.texture != NULL) {
		CFRelease(texture->metal.texture);
		texture->metal.texture = NULL;
	}
}

void kore_metal_texture_view_create(kore_gpu_device *device, kore_gpu_texture *texture, kore_gpu_texture_view *view) {
	view->texture = texture;
	view->format = texture->format;
	view->dimension = KORE_GPU_TEXTURE_VIEW_DIMENSION_2D;
	view->aspect = KORE_GPU_IMAGE_COPY_ASPECT_ALL;
	view->base_mip_level = 0;
	view->mip_level_count = texture->mip_level_count;
	view->base_array_layer = 0;
	view->array_layer_count = 1;
}

void kore_metal_texture_view_destroy(kore_gpu_texture_view *view) {
}

void kore_metal_texture_upload(kore_gpu_device *device, kore_gpu_texture *texture, const void *pixels, uint32_t width, uint32_t height) {
	kore_gpu_buffer staging_buffer;
	
	uint32_t bytes_per_pixel = kore_gpu_texture_format_byte_size(texture->format);
	uint64_t data_size = (uint64_t)width * height * bytes_per_pixel;
	
	kore_gpu_buffer_parameters staging_params = {
		.size = data_size,
		.usage_flags = KORE_GPU_BUFFER_USAGE_COPY_SRC | KORE_GPU_BUFFER_USAGE_CPU_WRITE,
	};
	kore_gpu_device_create_buffer(device, &staging_params, &staging_buffer);
	
	void *staging_ptr = kore_gpu_buffer_lock_all(&staging_buffer);
	memcpy(staging_ptr, pixels, data_size);
	kore_gpu_buffer_unlock_all(&staging_buffer);
	
	kore_gpu_command_list copy_list;
	kore_gpu_device_create_command_list(device, KORE_GPU_COMMAND_LIST_TYPE_COPY, &copy_list);
	
	kore_gpu_image_copy_buffer source = {
		.buffer = &staging_buffer,
		.offset = 0,
		.bytes_per_row = width * bytes_per_pixel,
		.rows_per_image = height,
	};
	kore_gpu_image_copy_texture dest = {
		.texture = texture,
		.mip_level = 0,
		.origin_x = 0,
		.origin_y = 0,
		.origin_z = 0,
	};
	kore_gpu_command_list_copy_buffer_to_texture(&copy_list, &source, &dest, width, height, 1);
	kore_gpu_device_execute_command_list(device, &copy_list);
	
	kore_gpu_buffer_destroy(&staging_buffer);
}
