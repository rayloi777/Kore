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
	void *vertex_buffer;
	void *index_buffer;
	uint64_t vertex_count;
	uint32_t index_count;
	bool allows_update;
	bool is_compacted;
} kore_metal_raytracing_volume;

typedef struct kore_metal_raytracing_hierarchy {
	void *acceleration_structure;
	void *instance_buffer;
	void *scratch_buffer;
	void *compacted_size_buffer;
	uint32_t instance_count;
	bool allows_update;
	bool is_compacted;
} kore_metal_raytracing_hierarchy;

#ifdef __cplusplus
}
#endif

#endif
