#include "kong.h"


#include <kore3/metal/buffer_functions.h>
#include <kore3/metal/commandlist_functions.h>
#include <kore3/metal/device_functions.h>
#include <kore3/metal/descriptorset_functions.h>
#include <kore3/metal/pipeline_functions.h>
#include <kore3/metal/texture_functions.h>
#include <kore3/util/align.h>

#include <assert.h>
#include <stdlib.h>

#import <MetalKit/MTKView.h>

uint32_t kong_vertex_in_buffer_usage_flags(void) {
	return KORE_METAL_BUFFER_USAGE_VERTEX;
}

void kong_create_buffer_vertex_in(kore_gpu_device * device, size_t count, vertex_in_buffer *buffer) {
	kore_gpu_buffer_parameters parameters;
	parameters.size = count * sizeof(vertex_in);
	parameters.usage_flags = KORE_GPU_BUFFER_USAGE_CPU_WRITE | kong_vertex_in_buffer_usage_flags();
	kore_gpu_device_create_buffer(device, &parameters, &buffer->buffer);
	buffer->count = count;
}

void kong_destroy_buffer_vertex_in(vertex_in_buffer *buffer) {
	kore_metal_buffer_destroy(&buffer->buffer);
}

vertex_in *kong_vertex_in_buffer_lock(vertex_in_buffer *buffer) {
	return (vertex_in *)kore_metal_buffer_lock_all(&buffer->buffer);
}

vertex_in *kong_vertex_in_buffer_try_to_lock(vertex_in_buffer *buffer) {
	return (vertex_in *)kore_metal_buffer_try_to_lock_all(&buffer->buffer);
}

void kong_vertex_in_buffer_unlock(vertex_in_buffer *buffer) {
	kore_metal_buffer_unlock(&buffer->buffer);
}

void kong_set_vertex_buffer_vertex_in(kore_gpu_command_list *list, vertex_in_buffer *buffer) {
	kore_metal_command_list_set_vertex_buffer(list, 0, &buffer->buffer.metal, 0, buffer->count * sizeof(vertex_in), sizeof(vertex_in));
}

static uint32_t mvp_vertex_table_index = UINT32_MAX;

static uint32_t mvp_fragment_table_index = UINT32_MAX;

static uint32_t mvp_compute_table_index = UINT32_MAX;

static uint32_t textures_vertex_table_index = UINT32_MAX;

static uint32_t textures_fragment_table_index = UINT32_MAX;

static uint32_t textures_compute_table_index = UINT32_MAX;

static kore_metal_render_pipeline pipeline;

static uint32_t _69_uniform_block_index;
void kong_set_render_pipeline_pipeline(kore_gpu_command_list *list) {
	kore_metal_command_list_set_render_pipeline(list, &pipeline);
	mvp_vertex_table_index = 1;
	mvp_fragment_table_index = 1;
	textures_vertex_table_index = 2;
	textures_fragment_table_index = 2;
}

uint32_t constants_type_buffer_usage_flags(void) {
	return 0u;
}

void constants_type_buffer_create(kore_gpu_device *device, kore_gpu_buffer *buffer, uint32_t count) {
	kore_gpu_buffer_parameters parameters;
	parameters.size = align_pow2(64, 256) * count;
	parameters.usage_flags = KORE_GPU_BUFFER_USAGE_CPU_WRITE | constants_type_buffer_usage_flags();
	kore_gpu_device_create_buffer(device, &parameters, buffer);
}

void constants_type_buffer_destroy(kore_gpu_buffer *buffer) {
	kore_gpu_buffer_destroy(buffer);
}

constants_type *constants_type_buffer_lock(kore_gpu_buffer *buffer, uint32_t index, uint32_t count) {
	return (constants_type *)kore_gpu_buffer_lock(buffer, index * align_pow2((int)sizeof(constants_type), 256), count * align_pow2((int)sizeof(constants_type), 256));
}

constants_type *constants_type_buffer_try_to_lock(kore_gpu_buffer *buffer, uint32_t index, uint32_t count) {
	return (constants_type *)kore_gpu_buffer_try_to_lock(buffer, index * align_pow2((int)sizeof(constants_type), 256), count * align_pow2((int)sizeof(constants_type), 256));
}

void constants_type_buffer_unlock(kore_gpu_buffer *buffer) {
	constants_type *data = (constants_type *)buffer->metal.locked_data;
	kore_gpu_buffer_unlock(buffer);
}

uint32_t tex_texture_usage_flags(void) {
	uint32_t usage = 0u;
	usage |= KORE_METAL_TEXTURE_USAGE_SAMPLE;
	return usage;
}

void kong_fill_mvp_set(kore_gpu_device *device, mvp_set *set) {
}

void kong_create_mvp_set(kore_gpu_device *device, const mvp_parameters *parameters, mvp_set *set) {
	id<MTLDevice> metal_device = (__bridge id<MTLDevice>)device->metal.device;

	MTLArgumentDescriptor* descriptor0 = [MTLArgumentDescriptor argumentDescriptor];
	descriptor0.index = 0;
	descriptor0.dataType = MTLDataTypePointer;

	id<MTLArgumentEncoder> argument_encoder = [metal_device newArgumentEncoderWithArguments: @[descriptor0]];

	kore_metal_device_create_descriptor_set_buffer(device, [argument_encoder encodedLength], &set->set.argument_buffer);

	id<MTLBuffer> metal_argument_buffer = (__bridge id<MTLBuffer>)set->set.argument_buffer.metal.buffer;

	[argument_encoder setArgumentBuffer:metal_argument_buffer offset:0];

	{
		id<MTLBuffer> buffer = (__bridge id<MTLBuffer>)parameters->constants->metal.buffer;
		[argument_encoder setBuffer: buffer offset: 0 atIndex: 0];
		set->constants = parameters->constants;
	}

}

void kong_destroy_mvp_set(mvp_set *set) {
}
void kong_update_mvp_set(mvp_set *set, mvp_set_update *updates, uint32_t updates_count) {
}

void kong_set_descriptor_set_mvp(kore_gpu_command_list *list, mvp_set *set) {
	kore_metal_descriptor_set_prepare_buffer(list, set->constants, 0, UINT32_MAX);


	kore_metal_command_list_set_descriptor_set(list, mvp_vertex_table_index, mvp_fragment_table_index, mvp_compute_table_index, &set->set, NULL, NULL, NULL, 0);
}

void kong_fill_textures_set(kore_gpu_device *device, textures_set *set) {
}

void kong_create_textures_set(kore_gpu_device *device, const textures_parameters *parameters, textures_set *set) {
	id<MTLDevice> metal_device = (__bridge id<MTLDevice>)device->metal.device;

	MTLArgumentDescriptor* descriptor0 = [MTLArgumentDescriptor argumentDescriptor];
	descriptor0.index = 0;
	descriptor0.dataType = MTLDataTypeTexture;

	MTLArgumentDescriptor* descriptor1 = [MTLArgumentDescriptor argumentDescriptor];
	descriptor1.index = 1;
	descriptor1.dataType = MTLDataTypeSampler;

	id<MTLArgumentEncoder> argument_encoder = [metal_device newArgumentEncoderWithArguments: @[descriptor0, descriptor1]];

	kore_metal_device_create_descriptor_set_buffer(device, [argument_encoder encodedLength], &set->set.argument_buffer);

	id<MTLBuffer> metal_argument_buffer = (__bridge id<MTLBuffer>)set->set.argument_buffer.metal.buffer;

	[argument_encoder setArgumentBuffer:metal_argument_buffer offset:0];

	{
		id<MTLTexture> texture = (__bridge id<MTLTexture>)parameters->tex.texture->metal.texture;
		id<MTLTexture> view = [texture newTextureViewWithPixelFormat:texture.pixelFormat textureType:MTLTextureType2D levels:NSMakeRange(parameters->tex.base_mip_level, parameters->tex.mip_level_count) slices:NSMakeRange(parameters->tex.base_array_layer, parameters->tex.array_layer_count)];
		[argument_encoder setTexture: view atIndex: 0];
		set->tex_view = (__bridge_retained void*)view;
		set->tex = parameters->tex;
	}

	{
		id<MTLSamplerState> sampler = (__bridge id<MTLSamplerState>)parameters->sam->metal.sampler;
		[argument_encoder setSamplerState: sampler atIndex: 1];
		set->sam = parameters->sam;
	}

}

void kong_destroy_textures_set(textures_set *set) {
}
void kong_update_textures_set(textures_set *set, textures_set_update *updates, uint32_t updates_count) {
}

void kong_set_descriptor_set_textures(kore_gpu_command_list *list, textures_set *set) {
	kore_metal_descriptor_set_prepare_texture(list, set->tex_view, false);


	kore_metal_command_list_set_descriptor_set(list, textures_vertex_table_index, textures_fragment_table_index, textures_compute_table_index, &set->set, NULL, NULL, NULL, 0);
}


void kong_init(kore_gpu_device *device) {
	kore_metal_render_pipeline_parameters pipeline_parameters = {0};

	pipeline_parameters.vertex.shader.function_name = "pos";
	pipeline_parameters.fragment.shader.function_name = "pix";
	pipeline_parameters.vertex.buffers[0].attributes[0].format = KORE_METAL_VERTEX_FORMAT_FLOAT32X3;
	pipeline_parameters.vertex.buffers[0].attributes[0].offset = 0;
	pipeline_parameters.vertex.buffers[0].attributes[0].shader_location = 0;
	pipeline_parameters.vertex.buffers[0].attributes[1].format = KORE_METAL_VERTEX_FORMAT_FLOAT32X2;
	pipeline_parameters.vertex.buffers[0].attributes[1].offset = 12;
	pipeline_parameters.vertex.buffers[0].attributes[1].shader_location = 1;
	pipeline_parameters.vertex.buffers[0].attributes_count = 2;
	pipeline_parameters.vertex.buffers[0].array_stride = 20;
	pipeline_parameters.vertex.buffers[0].step_mode = KORE_METAL_VERTEX_STEP_MODE_VERTEX;
	pipeline_parameters.vertex.buffers_count = 1;

	pipeline_parameters.primitive.topology = KORE_METAL_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipeline_parameters.primitive.strip_index_format = KORE_GPU_INDEX_FORMAT_UINT16;
	pipeline_parameters.primitive.front_face = KORE_METAL_FRONT_FACE_CW;
	pipeline_parameters.primitive.cull_mode = KORE_METAL_CULL_MODE_NONE;
	pipeline_parameters.primitive.unclipped_depth = false;

	pipeline_parameters.depth_stencil.format = KORE_GPU_TEXTURE_FORMAT_DEPTH32_FLOAT;
	pipeline_parameters.depth_stencil.depth_write_enabled = true;
	pipeline_parameters.depth_stencil.depth_compare = KORE_GPU_COMPARE_FUNCTION_LESS;
	pipeline_parameters.depth_stencil.stencil_front.compare = KORE_GPU_COMPARE_FUNCTION_ALWAYS;
	pipeline_parameters.depth_stencil.stencil_front.fail_op = KORE_METAL_STENCIL_OPERATION_KEEP;
	pipeline_parameters.depth_stencil.stencil_front.depth_fail_op = KORE_METAL_STENCIL_OPERATION_KEEP;
	pipeline_parameters.depth_stencil.stencil_front.pass_op = KORE_METAL_STENCIL_OPERATION_KEEP;
	pipeline_parameters.depth_stencil.stencil_back.compare = KORE_GPU_COMPARE_FUNCTION_ALWAYS;
	pipeline_parameters.depth_stencil.stencil_back.fail_op = KORE_METAL_STENCIL_OPERATION_KEEP;
	pipeline_parameters.depth_stencil.stencil_back.depth_fail_op = KORE_METAL_STENCIL_OPERATION_KEEP;
	pipeline_parameters.depth_stencil.stencil_back.pass_op = KORE_METAL_STENCIL_OPERATION_KEEP;
	pipeline_parameters.depth_stencil.stencil_read_mask = 0xffffffff;
	pipeline_parameters.depth_stencil.stencil_write_mask = 0xffffffff;
	pipeline_parameters.depth_stencil.depth_bias = 0;
	pipeline_parameters.depth_stencil.depth_bias_slope_scale = 0.0f;
	pipeline_parameters.depth_stencil.depth_bias_clamp = 0.0f;

	pipeline_parameters.multisample.count = 1;
	pipeline_parameters.multisample.mask = 0xffffffff;
	pipeline_parameters.multisample.alpha_to_coverage_enabled = false;

	pipeline_parameters.fragment.targets_count = 1;
	pipeline_parameters.fragment.targets[0].format = kore_gpu_device_framebuffer_format(device);
	pipeline_parameters.fragment.targets[0].write_mask = 0xf;

	pipeline_parameters.fragment.targets[0].blend.color.src_factor = KORE_METAL_BLEND_FACTOR_ONE;
	pipeline_parameters.fragment.targets[0].blend.color.dst_factor = KORE_METAL_BLEND_FACTOR_ZERO;
	pipeline_parameters.fragment.targets[0].blend.color.operation = KORE_METAL_BLEND_OPERATION_ADD;
	pipeline_parameters.fragment.targets[0].blend.alpha.src_factor = KORE_METAL_BLEND_FACTOR_ONE;
	pipeline_parameters.fragment.targets[0].blend.alpha.dst_factor = KORE_METAL_BLEND_FACTOR_ZERO;
	pipeline_parameters.fragment.targets[0].blend.alpha.operation = KORE_METAL_BLEND_OPERATION_ADD;

	kore_metal_render_pipeline_init(&device->metal, &pipeline, &pipeline_parameters);

}
