#ifndef KORE_METAL_RAYTRACING_STRUCTS_HEADER
#define KORE_METAL_RAYTRACING_STRUCTS_HEADER

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kore_metal_raytracing_volume {
	void *acceleration_structure;
	void *scratch_buffer;
	void *compacted_size_buffer;
} kore_metal_raytracing_volume;

typedef struct kore_metal_raytracing_hierarchy {
	void *acceleration_structure;
	void *instance_buffer;
	void *scratch_buffer;
} kore_metal_raytracing_hierarchy;

#ifdef __cplusplus
}
#endif

#endif
