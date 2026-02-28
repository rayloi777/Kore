#include <kore3/gpu/error.h>

#include <stdio.h>
#include <string.h>

static kore_gpu_error last_error = KORE_GPU_ERROR_SUCCESS;

const char *kore_gpu_error_to_string(kore_gpu_error error) {
	switch (error) {
		case KORE_GPU_ERROR_SUCCESS:
			return "Success";
		case KORE_GPU_ERROR_OUT_OF_MEMORY:
			return "Out of memory";
		case KORE_GPU_ERROR_INVALID_VALUE:
			return "Invalid value";
		case KORE_GPU_ERROR_INVALID_OPERATION:
			return "Invalid operation";
		case KORE_GPU_ERROR_DEVICE_LOST:
			return "Device lost";
		case KORE_GPU_ERROR_NOT_SUPPORTED:
			return "Not supported";
		case KORE_GPU_ERROR_VALIDATION_FAILED:
			return "Validation failed";
		case KORE_GPU_ERROR_SHADER_COMPILATION_FAILED:
			return "Shader compilation failed";
		case KORE_GPU_ERROR_PIPELINE_CREATION_FAILED:
			return "Pipeline creation failed";
		case KORE_GPU_ERROR_BUFFER_CREATION_FAILED:
			return "Buffer creation failed";
		case KORE_GPU_ERROR_TEXTURE_CREATION_FAILED:
			return "Texture creation failed";
		case KORE_GPU_ERROR_SAMPLER_CREATION_FAILED:
			return "Sampler creation failed";
		default:
			return "Unknown error";
	}
}

kore_gpu_error kore_gpu_device_get_last_error(void) {
	return last_error;
}

void kore_gpu_device_set_last_error(kore_gpu_error error) {
	last_error = error;
}

void kore_gpu_device_clear_last_error(void) {
	last_error = KORE_GPU_ERROR_SUCCESS;
}
