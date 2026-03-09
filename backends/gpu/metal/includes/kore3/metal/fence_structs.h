#ifndef KORE_METAL_FENCE_STRUCTS_HEADER
#define KORE_METAL_FENCE_STRUCTS_HEADER

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kore_metal_fence {
	void *event;
	uint64_t current_value;
} kore_metal_fence;

#ifdef __cplusplus
}
#endif

#endif
