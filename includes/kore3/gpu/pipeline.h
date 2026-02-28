#ifndef KORE_GPU_PIPELINE_HEADER
#define KORE_GPU_PIPELINE_HEADER

#include <kore3/global.h>

#include "api.h"

#if defined(KORE_DIRECT3D11)
#include <kore3/direct3d11/pipeline_structs.h>
#elif defined(KORE_DIRECT3D12)
#include <kore3/direct3d12/pipeline_structs.h>
#elif defined(KORE_METAL)
#include <kore3/metal/pipeline_structs.h>
#elif defined(KORE_OPENGL)
#include <kore3/opengl/pipeline_structs.h>
#elif defined(KORE_VULKAN)
#include <kore3/vulkan/pipeline_structs.h>
#elif defined(KORE_WEBGPU)
#include <kore3/webgpu/pipeline_structs.h>
#elif defined(KORE_KOMPJUTA)
#include <kore3/kompjuta/pipeline_structs.h>
#elif defined(KORE_CONSOLE)
#include <kore3/console/pipeline_structs.h>
#else
#error("Unknown GPU backend")
#endif

#include <kore3/gpu/buffer.h>
#include <kore3/gpu/commandlist.h>
#include <kore3/gpu/device.h>
#include <kore3/gpu/textureformat.h>
#include <kore3/gpu/sampler.h>

#include <kore3/math/vector.h>

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum kore_gpu_vertex_step_mode {
	KORE_GPU_VERTEX_STEP_MODE_VERTEX,
	KORE_GPU_VERTEX_STEP_MODE_INSTANCE
} kore_gpu_vertex_step_mode;

typedef enum kore_gpu_vertex_format {
	KORE_GPU_VERTEX_FORMAT_UINT8X2,
	KORE_GPU_VERTEX_FORMAT_UINT8X4,
	KORE_GPU_VERTEX_FORMAT_SINT8X2,
	KORE_GPU_VERTEX_FORMAT_SINT8X4,
	KORE_GPU_VERTEX_FORMAT_UNORM8X2,
	KORE_GPU_VERTEX_FORMAT_UNORM8X4,
	KORE_GPU_VERTEX_FORMAT_SNORM8X2,
	KORE_GPU_VERTEX_FORMAT_SNORM8X4,
	KORE_GPU_VERTEX_FORMAT_UINT16X2,
	KORE_GPU_VERTEX_FORMAT_UINT16X4,
	KORE_GPU_VERTEX_FORMAT_SINT16X2,
	KORE_GPU_VERTEX_FORMAT_SINT16X4,
	KORE_GPU_VERTEX_FORMAT_UNORM16X2,
	KORE_GPU_VERTEX_FORMAT_UNORM16X4,
	KORE_GPU_VERTEX_FORMAT_SNORM16X2,
	KORE_GPU_VERTEX_FORMAT_SNORM16X4,
	KORE_GPU_VERTEX_FORMAT_FLOAT16X2,
	KORE_GPU_VERTEX_FORMAT_FLOAT16X4,
	KORE_GPU_VERTEX_FORMAT_FLOAT32,
	KORE_GPU_VERTEX_FORMAT_FLOAT32X2,
	KORE_GPU_VERTEX_FORMAT_FLOAT32X3,
	KORE_GPU_VERTEX_FORMAT_FLOAT32X4,
	KORE_GPU_VERTEX_FORMAT_UINT32,
	KORE_GPU_VERTEX_FORMAT_UINT32X2,
	KORE_GPU_VERTEX_FORMAT_UINT32X3,
	KORE_GPU_VERTEX_FORMAT_UINT32X4,
	KORE_GPU_VERTEX_FORMAT_SINT32,
	KORE_GPU_VERTEX_FORMAT_SINT32X2,
	KORE_GPU_VERTEX_FORMAT_SINT32X3,
	KORE_GPU_VERTEX_FORMAT_SINT32X4,
	KORE_GPU_VERTEX_FORMAT_UNORM10_10_10_2
} kore_gpu_vertex_format;

#define KORE_GPU_MAX_VERTEX_ATTRIBUTES 32

typedef struct kore_gpu_vertex_attribute {
	kore_gpu_vertex_format format;
	uint64_t              offset;
	uint32_t              shader_location;
} kore_gpu_vertex_attribute;

#define KORE_GPU_MAX_VERTEX_BUFFERS 16

typedef struct kore_gpu_vertex_buffer_layout {
	uint64_t                  array_stride;
	kore_gpu_vertex_step_mode step_mode;
	kore_gpu_vertex_attribute attributes[KORE_GPU_MAX_VERTEX_ATTRIBUTES];
	size_t                    attributes_count;
} kore_gpu_vertex_buffer_layout;

typedef struct kore_gpu_shader {
	const char *function_name;
} kore_gpu_shader;

typedef struct kore_gpu_vertex_state {
	kore_gpu_shader                shader;
	kore_gpu_vertex_buffer_layout buffers[KORE_GPU_MAX_VERTEX_BUFFERS];
	size_t                        buffers_count;
} kore_gpu_vertex_state;

typedef enum kore_gpu_primitive_topology {
	KORE_GPU_PRIMITIVE_TOPOLOGY_POINT_LIST,
	KORE_GPU_PRIMITIVE_TOPOLOGY_LINE_LIST,
	KORE_GPU_PRIMITIVE_TOPOLOGY_LINE_STRIP,
	KORE_GPU_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	KORE_GPU_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP
} kore_gpu_primitive_topology;

typedef enum kore_gpu_front_face {
	KORE_GPU_FRONT_FACE_CCW,
	KORE_GPU_FRONT_FACE_CW
} kore_gpu_front_face;

typedef enum kore_gpu_cull_mode {
	KORE_GPU_CULL_MODE_NONE,
	KORE_GPU_CULL_MODE_FRONT,
	KORE_GPU_CULL_MODE_BACK
} kore_gpu_cull_mode;

typedef struct kore_gpu_primitive_state {
	kore_gpu_primitive_topology topology;
	kore_gpu_index_format        strip_index_format;
	kore_gpu_front_face          front_face;
	kore_gpu_cull_mode           cull_mode;
	bool                         unclipped_depth;
} kore_gpu_primitive_state;

typedef enum kore_gpu_stencil_operation {
	KORE_GPU_STENCIL_OPERATION_KEEP,
	KORE_GPU_STENCIL_OPERATION_ZERO,
	KORE_GPU_STENCIL_OPERATION_REPLACE,
	KORE_GPU_STENCIL_OPERATION_INVERT,
	KORE_GPU_STENCIL_OPERATION_INCREMENT_CLAMP,
	KORE_GPU_STENCIL_OPERATION_DECREMENT_CLAMP,
	KORE_GPU_STENCIL_OPERATION_INCREMENT_WRAP,
	KORE_GPU_STENCIL_OPERATION_DECREMENT_WRAP
} kore_gpu_stencil_operation;

typedef struct kore_gpu_stencil_face_state {
	kore_gpu_compare_function    compare;
	kore_gpu_stencil_operation fail_op;
	kore_gpu_stencil_operation depthfail_op;
	kore_gpu_stencil_operation pass_op;
} kore_gpu_stencil_face_state;

typedef struct kore_gpu_depth_stencil_state {
	kore_gpu_texture_format    format;
	bool                       depth_write_enabled;
	kore_gpu_compare_function  depth_compare;
	kore_gpu_stencil_face_state stencil_front;
	kore_gpu_stencil_face_state stencil_back;
	uint32_t                   stencil_read_mask;
	uint32_t                   stencil_write_mask;
	int32_t                    depth_bias;
	float                      depth_bias_slope_scale;
	float                      depth_bias_clamp;
} kore_gpu_depth_stencil_state;

typedef struct kore_gpu_multisample_state {
	uint32_t count;
	uint32_t mask;
	bool     alpha_to_coverage_enabled;
} kore_gpu_multisample_state;

typedef enum kore_gpu_blend_operation {
	KORE_GPU_BLEND_OPERATION_ADD,
	KORE_GPU_BLEND_OPERATION_SUBTRACT,
	KORE_GPU_BLEND_OPERATION_REVERSE_SUBTRACT,
	KORE_GPU_BLEND_OPERATION_MIN,
	KORE_GPU_BLEND_OPERATION_MAX
} kore_gpu_blend_operation;

typedef enum kore_gpu_blend_factor {
	KORE_GPU_BLEND_FACTOR_ZERO,
	KORE_GPU_BLEND_FACTOR_ONE,
	KORE_GPU_BLEND_FACTOR_SRC,
	KORE_GPU_BLEND_FACTOR_ONE_MINUS_SRC,
	KORE_GPU_BLEND_FACTOR_SRC_ALPHA,
	KORE_GPU_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
	KORE_GPU_BLEND_FACTOR_DST,
	KORE_GPU_BLEND_FACTOR_ONE_MINUS_DST,
	KORE_GPU_BLEND_FACTOR_DST_ALPHA,
	KORE_GPU_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
	KORE_GPU_BLEND_FACTOR_SRC_ALPHA_SATURATED,
	KORE_GPU_BLEND_FACTOR_CONSTANT,
	KORE_GPU_BLEND_FACTOR_ONE_MINUS_CONSTANT
} kore_gpu_blend_factor;

typedef struct kore_gpu_blend_component {
	kore_gpu_blend_operation operation;
	kore_gpu_blend_factor    src_factor;
	kore_gpu_blend_factor    dst_factor;
} kore_gpu_blend_component;

typedef struct kore_gpu_blend_state {
	kore_gpu_blend_component color;
	kore_gpu_blend_component alpha;
} kore_gpu_blend_state;

typedef enum kore_gpu_color_write_flags {
	KORE_GPU_COLOR_WRITE_FLAGS_RED   = 0x1,
	KORE_GPU_COLOR_WRITE_FLAGS_GREEN = 0x2,
	KORE_GPU_COLOR_WRITE_FLAGS_BLUE  = 0x4,
	KORE_GPU_COLOR_WRITE_FLAGS_ALPHA = 0x8,
	KORE_GPU_COLOR_WRITE_FLAGS_ALL   = 0xF
} kore_gpu_color_write_flags;

#define KORE_GPU_MAX_COLOR_TARGETS 8

typedef struct kore_gpu_color_target_state {
	kore_gpu_texture_format format;
	kore_gpu_blend_state   blend;
	uint32_t               write_mask;
} kore_gpu_color_target_state;

typedef struct kore_gpu_fragment_state {
	kore_gpu_shader             shader;
	kore_gpu_color_target_state targets[KORE_GPU_MAX_COLOR_TARGETS];
	size_t                      targets_count;
} kore_gpu_fragment_state;

typedef struct kore_gpu_render_pipeline_parameters {
	kore_gpu_vertex_state        vertex;
	kore_gpu_primitive_state     primitive;
	kore_gpu_depth_stencil_state depth_stencil;
	kore_gpu_multisample_state   multisample;
	kore_gpu_fragment_state      fragment;
} kore_gpu_render_pipeline_parameters;

typedef struct kore_gpu_render_pipeline {
	KORE_GPU_IMPL(render_pipeline);
} kore_gpu_render_pipeline;

typedef struct kore_gpu_compute_pipeline_parameters {
	kore_gpu_shader shader;
} kore_gpu_compute_pipeline_parameters;

typedef struct kore_gpu_compute_pipeline {
	KORE_GPU_IMPL(compute_pipeline);
} kore_gpu_compute_pipeline;

KORE_FUNC void kore_gpu_render_pipeline_init(kore_gpu_device *device, kore_gpu_render_pipeline *pipeline, const kore_gpu_render_pipeline_parameters *parameters);

KORE_FUNC void kore_gpu_render_pipeline_destroy(kore_gpu_render_pipeline *pipeline);

KORE_FUNC void kore_gpu_compute_pipeline_init(kore_gpu_device *device, kore_gpu_compute_pipeline *pipeline, const kore_gpu_compute_pipeline_parameters *parameters);

KORE_FUNC void kore_gpu_compute_pipeline_destroy(kore_gpu_compute_pipeline *pipeline);

#ifdef __cplusplus
}
#endif

#endif
