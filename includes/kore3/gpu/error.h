#ifndef KORE_GPU_ERROR_HEADER
#define KORE_GPU_ERROR_HEADER

#include <kore3/global.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum kore_gpu_error {
    KORE_GPU_ERROR_SUCCESS = 0,
    KORE_GPU_ERROR_OUT_OF_MEMORY,
    KORE_GPU_ERROR_INVALID_VALUE,
    KORE_GPU_ERROR_INVALID_OPERATION,
    KORE_GPU_ERROR_DEVICE_LOST,
    KORE_GPU_ERROR_NOT_SUPPORTED,
    KORE_GPU_ERROR_VALIDATION_FAILED,
    KORE_GPU_ERROR_SHADER_COMPILATION_FAILED,
    KORE_GPU_ERROR_PIPELINE_CREATION_FAILED,
    KORE_GPU_ERROR_BUFFER_CREATION_FAILED,
    KORE_GPU_ERROR_TEXTURE_CREATION_FAILED,
    KORE_GPU_ERROR_SAMPLER_CREATION_FAILED,
} kore_gpu_error;

KORE_FUNC const char *kore_gpu_error_to_string(kore_gpu_error error);

KORE_FUNC kore_gpu_error kore_gpu_device_get_last_error(void);

KORE_FUNC void kore_gpu_device_set_last_error(kore_gpu_error error);

KORE_FUNC void kore_gpu_device_clear_last_error(void);

#ifdef __cplusplus
}
#endif

#endif
