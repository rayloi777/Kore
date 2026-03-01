#ifndef KONG_INTEGRATION_HEADER
#define KONG_INTEGRATION_HEADER

#include <kore3/gpu/device.h>
#include <kore3/gpu/sampler.h>
#include <kore3/metal/descriptorset_structs.h>
#include <kore3/metal/pipeline_structs.h>
#include <kore3/math/matrix.h>
#include <kore3/math/vector.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
#define KONG_PACK_START __pragma(pack(push, 1))
#define KONG_PACK_END __pragma(pack(pop))
#define KONG_PACK
#else
#define KONG_PACK_START
#define KONG_PACK_END
#define KONG_PACK __attribute__((packed))
#endif

void kong_init(kore_gpu_device *device);

typedef struct constants_type {
	kore_matrix4x4 mvp;
} constants_type;

uint32_t constants_type_buffer_usage_flags(void);
void constants_type_buffer_create(kore_gpu_device *device, kore_gpu_buffer *buffer, uint32_t count);
void constants_type_buffer_destroy(kore_gpu_buffer *buffer);
constants_type *constants_type_buffer_lock(kore_gpu_buffer *buffer, uint32_t index, uint32_t count);
constants_type *constants_type_buffer_try_to_lock(kore_gpu_buffer *buffer, uint32_t index, uint32_t count);
void constants_type_buffer_unlock(kore_gpu_buffer *buffer);
uint32_t tex_texture_usage_flags(void);

typedef struct mvp_parameters {
	kore_gpu_buffer *constants;
} mvp_parameters;

typedef struct mvp_set {
	kore_metal_descriptor_set set;

	kore_gpu_buffer *constants;
} mvp_set;

void kong_create_mvp_set(kore_gpu_device *device, const mvp_parameters *parameters, mvp_set *set);
void kong_destroy_mvp_set(mvp_set *set);
void kong_set_descriptor_set_mvp(kore_gpu_command_list *list, mvp_set *set);

typedef struct mvp_set_update {
	enum {
		MVP_SET_UPDATE_CONSTANTS,
	} kind;
	union {
		kore_gpu_buffer *constants;
	};
} mvp_set_update;

void kong_update_mvp_set(mvp_set *set, mvp_set_update *updates, uint32_t updates_count);
typedef struct textures_parameters {
	kore_gpu_texture_view tex;
	kore_gpu_sampler *sam;
} textures_parameters;

typedef struct textures_set {
	kore_metal_descriptor_set set;

	kore_gpu_texture_view tex;
	void *tex_view;
	kore_gpu_sampler *sam;
} textures_set;

void kong_create_textures_set(kore_gpu_device *device, const textures_parameters *parameters, textures_set *set);
void kong_destroy_textures_set(textures_set *set);
void kong_set_descriptor_set_textures(kore_gpu_command_list *list, textures_set *set);

typedef struct textures_set_update {
	enum {
		TEXTURES_SET_UPDATE_TEX,
		TEXTURES_SET_UPDATE_SAM,
	} kind;
	union {
		kore_gpu_texture_view tex;
		kore_gpu_sampler *sam;
	};
} textures_set_update;

void kong_update_textures_set(textures_set *set, textures_set_update *updates, uint32_t updates_count);

KONG_PACK_START
typedef struct KONG_PACK vertex_in {
	kore_float3 pos;
	kore_float2 uv;
} vertex_in;
KONG_PACK_END

typedef struct vertex_in_buffer {
	kore_gpu_buffer buffer;
	size_t count;
} vertex_in_buffer;

uint32_t kong_vertex_in_buffer_usage_flags(void);
void kong_create_buffer_vertex_in(kore_gpu_device * device, size_t count, vertex_in_buffer *buffer);
void kong_destroy_buffer_vertex_in(vertex_in_buffer *buffer);
vertex_in *kong_vertex_in_buffer_lock(vertex_in_buffer *buffer);
vertex_in *kong_vertex_in_buffer_try_to_lock(vertex_in_buffer *buffer);
void kong_vertex_in_buffer_unlock(vertex_in_buffer *buffer);
void kong_set_vertex_buffer_vertex_in(kore_gpu_command_list *list, vertex_in_buffer *buffer);

void kong_set_render_pipeline_pipeline(kore_gpu_command_list *list);

#ifdef __cplusplus
}
#endif

#endif
