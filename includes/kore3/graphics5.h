#ifndef KORE_GRAPHICS5_H
#define KORE_GRAPHICS5_H

#include <kore3/global.h>

typedef enum {
	KORE_G5_CLEAR_COLOR = 0
} kore_g5_clear_action;

typedef enum {
	KORE_G5_SHADER_TYPE_VERTEX,
	KORE_G5_SHADER_TYPE_FRAGMENT
} kore_g5_shader_type;

typedef enum {
	KORE_G5_VERTEX_DATA_FLOAT3
} kore_g5_vertex_data;

typedef enum {
	KORE_G5_INDEX_BUFFER_FORMAT_32BIT
} kore_g5_index_buffer_format;

typedef enum {
	KORE_G5_RENDER_TARGET_FORMAT_32BIT
} kore_g5_render_target_format;

typedef struct kore_g5_render_target {
	kore_gpu_texture *texture;
	int index;
} kore_g5_render_target_t;

typedef struct {
	int nothing;
} kore_g5_command_list_t;

typedef struct {
	const char *function_name;
} kore_g5_shader_t;

typedef struct {
	int nothing;
} kore_g5_pipeline_t;

typedef struct {
	int nothing;
} kore_g5_vertex_buffer_t;

typedef struct {
	int nothing;
} kore_g5_index_buffer_t;

typedef struct {
	int nothing;
} kore_g5_vertex_structure_t;

void kore_g4_vertex_structure_init(kore_g5_vertex_structure_t *structure);
void kore_g4_vertex_structure_add(kore_g5_vertex_structure_t *structure, const char *name, kore_g5_vertex_data data);

void kore_g5_begin(kore_g5_render_target_t *render_target, int index);
void kore_g5_end(int index);
void kore_g5_swap_buffers(void);

void kore_g5_command_list_init(kore_g5_command_list_t *cmd);
void kore_g5_command_list_begin(kore_g5_command_list_t *cmd);
void kore_g5_command_list_end(kore_g5_command_list_t *cmd);
void kore_g5_command_list_execute_and_wait(kore_g5_command_list_t *cmd);
void kore_g5_command_list_framebuffer_to_render_target_barrier(kore_g5_command_list_t *cmd, kore_g5_render_target_t *target);
void kore_g5_command_list_render_target_to_framebuffer_barrier(kore_g5_command_list_t *cmd, kore_g5_render_target_t *target);
void kore_g5_command_list_set_render_targets(kore_g5_command_list_t *cmd, kore_g5_render_target_t **targets, int count);
void kore_g5_command_list_clear(kore_g5_command_list_t *cmd, kore_g5_render_target_t *target, kore_g5_clear_action action, int index, float r, float g);
void kore_g5_command_list_set_pipeline(kore_g5_command_list_t *cmd, kore_g5_pipeline_t *pipeline);
void kore_g5_command_list_set_pipeline_layout(kore_g5_command_list_t *cmd);
void kore_g5_command_list_set_vertex_buffers(kore_g5_command_list_t *cmd, kore_g5_vertex_buffer_t **buffers, int *offsets, int count);
void kore_g5_command_list_set_index_buffer(kore_g5_command_list_t *cmd, kore_g5_index_buffer_t *buffer);
void kore_g5_command_list_draw_indexed_vertices(kore_g5_command_list_t *cmd);

void kore_g5_shader_init(kore_g5_shader_t *shader, const uint8_t *data, size_t size, kore_g5_shader_type type);

void kore_g5_pipeline_init(kore_g5_pipeline_t *pipeline);
void kore_g5_pipeline_compile(kore_g5_pipeline_t *pipeline);

void kore_g5_render_target_init(kore_g5_render_target_t *target, int width, int height, int depth, bool render_target, kore_g5_render_target_format format, int depth_stencil, int index);

void kore_g5_vertex_buffer_init(kore_g5_vertex_buffer_t *buffer, int count, kore_g5_vertex_structure_t *structure, bool cpu_access, int instance_count);
float *kore_g5_vertex_buffer_lock_all(kore_g5_vertex_buffer_t *buffer);
void kore_g5_vertex_buffer_unlock_all(kore_g5_vertex_buffer_t *buffer);
void kore_g5_command_list_upload_vertex_buffer(kore_g5_command_list_t *cmd, kore_g5_vertex_buffer_t *buffer);

void kore_g5_index_buffer_init(kore_g5_index_buffer_t *buffer, int count, kore_g5_index_buffer_format format, bool cpu_access);
int *kore_g5_index_buffer_lock_all(kore_g5_index_buffer_t *buffer);
void kore_g5_index_buffer_unlock_all(kore_g5_index_buffer_t *buffer);
void kore_g5_command_list_upload_index_buffer(kore_g5_command_list_t *cmd, kore_g5_index_buffer_t *buffer);

#endif
