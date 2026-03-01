#ifndef KORE_GPU_COMMON_HEADER
#define KORE_GPU_COMMON_HEADER

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(KORE_METAL) || defined(KORE_VULKAN)
    #define KORE_GPU_NDC_Z_ZERO_ONE 1
    #define KORE_GPU_FRAME_COUNT 2
    #define KORE_GPU_EXECUTION_FENCE_COUNT 8
    #define KORE_GPU_MAX_BUFFER_RANGES 16
#else
    #define KORE_GPU_NDC_Z_ZERO_ONE 0
#endif

#ifdef __cplusplus
}
#endif

#endif
