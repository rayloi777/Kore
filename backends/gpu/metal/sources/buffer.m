#include <kore3/metal/buffer_functions.h>

#include <kore3/gpu/buffer.h>
#include <kore3/gpu/device.h>
#include <string.h>

static uint64_t find_max_execution_index_all(kore_gpu_buffer *buffer) {
	uint64_t max_execution_index = 0;

	for (uint32_t range_index = 0; range_index < buffer->metal.ranges_count; ++range_index) {
		uint64_t execution_index = buffer->metal.ranges[range_index].execution_index;
		if (execution_index > max_execution_index) {
			max_execution_index = execution_index;
		}
	}

	return max_execution_index;
}

static uint64_t find_max_execution_index(kore_gpu_buffer *buffer, uint64_t offset, uint64_t size) {
	uint64_t max_execution_index = 0;

	for (uint32_t range_index = 0; range_index < buffer->metal.ranges_count; ++range_index) {
		kore_metal_buffer_range range = buffer->metal.ranges[range_index];

		if (range.size == UINT64_MAX || (offset >= range.offset && offset < range.offset + range.size) ||
		    (offset + size > range.offset && offset + size <= range.offset + range.size)) {
			uint64_t execution_index = buffer->metal.ranges[range_index].execution_index;
			if (execution_index > max_execution_index) {
				max_execution_index = execution_index;
			}
		}
	}

	return max_execution_index;
}

void kore_metal_buffer_set_name(kore_gpu_buffer *buffer, const char *name) {
	id<MTLBuffer> metal_buffer = (__bridge id<MTLBuffer>)buffer->metal.buffer;
	metal_buffer.label = [NSString stringWithUTF8String:name];
}

void kore_metal_buffer_destroy(kore_gpu_buffer *buffer) {
	CFRelease(buffer->metal.buffer);
	buffer->metal.buffer = NULL;
}

void *kore_metal_buffer_try_to_lock_all(kore_gpu_buffer *buffer) {
	if (find_completed_execution(buffer->metal.device) >= find_max_execution_index_all(buffer)) {
		id<MTLBuffer> metal_buffer = (__bridge id<MTLBuffer>)buffer->metal.buffer;
		buffer->metal.locked_data  = (void *)[metal_buffer contents];
		return buffer->metal.locked_data;
	}
	else {
		return NULL;
	}
}

void *kore_metal_buffer_lock_all(kore_gpu_buffer *buffer) {
	wait_for_execution(buffer->metal.device, find_max_execution_index_all(buffer));

	id<MTLBuffer> metal_buffer = (__bridge id<MTLBuffer>)buffer->metal.buffer;
	buffer->metal.locked_data  = (void *)[metal_buffer contents];
	return buffer->metal.locked_data;
}

void *kore_metal_buffer_try_to_lock(kore_gpu_buffer *buffer, uint64_t offset, uint64_t size) {
	if (find_completed_execution(buffer->metal.device) >= find_max_execution_index(buffer, offset, size)) {
		id<MTLBuffer> metal_buffer = (__bridge id<MTLBuffer>)buffer->metal.buffer;
		uint8_t      *data         = (uint8_t *)[metal_buffer contents];
		buffer->metal.locked_data  = &data[offset];
		return buffer->metal.locked_data;
	}
	else {
		return NULL;
	}
}

void *kore_metal_buffer_lock(kore_gpu_buffer *buffer, uint64_t offset, uint64_t size) {
	wait_for_execution(buffer->metal.device, find_max_execution_index(buffer, offset, size));

	id<MTLBuffer> metal_buffer = (__bridge id<MTLBuffer>)buffer->metal.buffer;
	uint8_t      *data         = (uint8_t *)[metal_buffer contents];
	buffer->metal.locked_data  = &data[offset];
	return buffer->metal.locked_data;
}

void kore_metal_buffer_unlock(kore_gpu_buffer *buffer) {
#ifndef KORE_APPLE_SOC
	if (buffer->metal.host_visible) {
		id<MTLBuffer> metal_buffer = (__bridge id<MTLBuffer>)buffer->metal.buffer;
		[metal_buffer didModifyRange:NSMakeRange(0, buffer->metal.size)];
	}
#endif
	buffer->metal.locked_data = NULL;
}

void kore_metal_buffer_unlock_all(kore_gpu_buffer *buffer) {
	buffer->metal.locked_data = NULL;
}

void kore_metal_buffer_upload(kore_gpu_device *device, const void *data, uint64_t size, uint32_t usage_flags, kore_gpu_buffer *buffer) {
	void *ptr = kore_metal_buffer_lock_all(buffer);
	memcpy(ptr, data, size);
	kore_metal_buffer_unlock_all(buffer);
}
