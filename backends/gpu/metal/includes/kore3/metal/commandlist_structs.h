#ifndef KORE_METAL_COMMANDLIST_STRUCTS_HEADER
#define KORE_METAL_COMMANDLIST_STRUCTS_HEADER

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct kore_metal_device;
struct kore_metal_texture;
struct kore_metal_compute_pipeline;
struct kore_metal_ray_pipeline;
struct kore_metal_render_pipeline;
struct kore_metal_descriptor_set;
struct kore_gpu_query_set;
struct kore_metal_buffer;

typedef struct kore_metal_buffer_access {
	struct kore_metal_buffer *buffer;
	uint64_t                  offset;
	uint64_t                  size;
} kore_metal_buffer_access;

#define KORE_METAL_COMMAND_LIST_MAX_QUEUED_BUFFER_ACCESSES 256

typedef struct kore_metal_command_list {
	void *command_queue;
	void *command_buffer;
	void *render_command_encoder;
	void *compute_command_encoder;
	void *blit_command_encoder;
	void *index_buffer;
	bool  sixteen_bit_indices;

	struct kore_metal_render_pipeline *current_render_pipeline;

	kore_metal_buffer_access queued_buffer_accesses[KORE_METAL_COMMAND_LIST_MAX_QUEUED_BUFFER_ACCESSES];
	uint32_t                 queued_buffer_accesses_count;
} kore_metal_command_list;

#ifdef __cplusplus
}
#endif

#endif
