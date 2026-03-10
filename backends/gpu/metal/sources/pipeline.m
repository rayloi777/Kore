#include <kore3/metal/pipeline_functions.h>
#include <kore3/metal/pipeline_structs.h>

#include "metalunit.h"

#include <kore3/log.h>

static MTLBlendFactor convert_blending_factor(kore_metal_blend_factor factor) {
	switch (factor) {
	case KORE_METAL_BLEND_FACTOR_ONE:
		return MTLBlendFactorOne;
	case KORE_METAL_BLEND_FACTOR_ZERO:
		return MTLBlendFactorZero;
	case KORE_METAL_BLEND_FACTOR_SRC_ALPHA:
		return MTLBlendFactorSourceAlpha;
	case KORE_METAL_BLEND_FACTOR_DST_ALPHA:
		return MTLBlendFactorDestinationAlpha;
	case KORE_METAL_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
		return MTLBlendFactorOneMinusSourceAlpha;
	case KORE_METAL_BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
		return MTLBlendFactorOneMinusDestinationAlpha;
	case KORE_METAL_BLEND_FACTOR_SRC:
		return MTLBlendFactorSourceColor;
	case KORE_METAL_BLEND_FACTOR_DST:
		return MTLBlendFactorDestinationColor;
	case KORE_METAL_BLEND_FACTOR_ONE_MINUS_SRC:
		return MTLBlendFactorOneMinusSourceColor;
	case KORE_METAL_BLEND_FACTOR_ONE_MINUS_DST:
		return MTLBlendFactorOneMinusDestinationColor;
	case KORE_METAL_BLEND_FACTOR_CONSTANT:
		return MTLBlendFactorBlendColor;
	case KORE_METAL_BLEND_FACTOR_ONE_MINUS_CONSTANT:
		return MTLBlendFactorOneMinusBlendColor;
	case KORE_METAL_BLEND_FACTOR_SRC_ALPHA_SATURATED:
		return MTLBlendFactorSourceAlphaSaturated;
	}
}

static MTLBlendOperation convert_blending_operation(kore_metal_blend_operation op) {
	switch (op) {
	case KORE_METAL_BLEND_OPERATION_ADD:
		return MTLBlendOperationAdd;
	case KORE_METAL_BLEND_OPERATION_SUBTRACT:
		return MTLBlendOperationSubtract;
	case KORE_METAL_BLEND_OPERATION_REVERSE_SUBTRACT:
		return MTLBlendOperationReverseSubtract;
	case KORE_METAL_BLEND_OPERATION_MIN:
		return MTLBlendOperationMin;
	case KORE_METAL_BLEND_OPERATION_MAX:
		return MTLBlendOperationMax;
	}
}

static uint32_t vertex_attribute_size(kore_metal_vertex_format format) {
	switch (format) {
	case KORE_METAL_VERTEX_FORMAT_UINT8X2:
		return 2;
	case KORE_METAL_VERTEX_FORMAT_UINT8X4:
		return 4;
	case KORE_METAL_VERTEX_FORMAT_SINT8X2:
		return 2;
	case KORE_METAL_VERTEX_FORMAT_SINT8X4:
		return 4;
	case KORE_METAL_VERTEX_FORMAT_UNORM8X2:
		return 2;
	case KORE_METAL_VERTEX_FORMAT_UNORM8X4:
		return 4;
	case KORE_METAL_VERTEX_FORMAT_SNORM8X2:
		return 2;
	case KORE_METAL_VERTEX_FORMAT_SNORM8X4:
		return 4;
	case KORE_METAL_VERTEX_FORMAT_UINT16X2:
		return 4;
	case KORE_METAL_VERTEX_FORMAT_UINT16X4:
		return 8;
	case KORE_METAL_VERTEX_FORMAT_SINT16X2:
		return 4;
	case KORE_METAL_VERTEX_FORMAT_SINT16X4:
		return 8;
	case KORE_METAL_VERTEX_FORMAT_UNORM16X2:
		return 4;
	case KORE_METAL_VERTEX_FORMAT_UNORM16X4:
		return 8;
	case KORE_METAL_VERTEX_FORMAT_SNORM16X2:
		return 4;
	case KORE_METAL_VERTEX_FORMAT_SNORM16X4:
		return 8;
	case KORE_METAL_VERTEX_FORMAT_FLOAT16X2:
		return 4;
	case KORE_METAL_VERTEX_FORMAT_FLOAT16X4:
		return 8;
	case KORE_METAL_VERTEX_FORMAT_FLOAT32:
		return 4;
	case KORE_METAL_VERTEX_FORMAT_FLOAT32X2:
		return 8;
	case KORE_METAL_VERTEX_FORMAT_FLOAT32X3:
		return 12;
	case KORE_METAL_VERTEX_FORMAT_FLOAT32X4:
		return 16;
	case KORE_METAL_VERTEX_FORMAT_UINT32:
		return 4;
	case KORE_METAL_VERTEX_FORMAT_UINT32X2:
		return 8;
	case KORE_METAL_VERTEX_FORMAT_UINT32X3:
		return 12;
	case KORE_METAL_VERTEX_FORMAT_UINT32X4:
		return 16;
	case KORE_METAL_VERTEX_FORMAT_SINT32:
		return 4;
	case KORE_METAL_VERTEX_FORMAT_SINT32X2:
		return 8;
	case KORE_METAL_VERTEX_FORMAT_SINT32X3:
		return 12;
	case KORE_METAL_VERTEX_FORMAT_SINT32X4:
		return 16;
	case KORE_METAL_VERTEX_FORMAT_UNORM10_10_10_2:
		return 4;
	}

	return 0;
}

static MTLCompareFunction convert_compare(kore_gpu_compare_function func) {
	switch (func) {
	case KORE_GPU_COMPARE_FUNCTION_UNDEFINED:
	case KORE_GPU_COMPARE_FUNCTION_NEVER:
		return MTLCompareFunctionNever;
	case KORE_GPU_COMPARE_FUNCTION_LESS:
		return MTLCompareFunctionLess;
	case KORE_GPU_COMPARE_FUNCTION_EQUAL:
		return MTLCompareFunctionEqual;
	case KORE_GPU_COMPARE_FUNCTION_LESS_EQUAL:
		return MTLCompareFunctionLessEqual;
	case KORE_GPU_COMPARE_FUNCTION_GREATER:
		return MTLCompareFunctionGreater;
	case KORE_GPU_COMPARE_FUNCTION_NOT_EQUAL:
		return MTLCompareFunctionNotEqual;
	case KORE_GPU_COMPARE_FUNCTION_GREATER_EQUAL:
		return MTLCompareFunctionGreaterEqual;
	case KORE_GPU_COMPARE_FUNCTION_ALWAYS:
		return MTLCompareFunctionAlways;
	}
}

static MTLPrimitiveType convert_primitive_topology(kore_metal_primitive_topology topology) {
	switch (topology) {
	case KORE_METAL_PRIMITIVE_TOPOLOGY_POINT_LIST:
		return MTLPrimitiveTypePoint;
	case KORE_METAL_PRIMITIVE_TOPOLOGY_LINE_LIST:
		return MTLPrimitiveTypeLine;
	case KORE_METAL_PRIMITIVE_TOPOLOGY_LINE_STRIP:
		return MTLPrimitiveTypeLineStrip;
	case KORE_METAL_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
		return MTLPrimitiveTypeTriangle;
	case KORE_METAL_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
		return MTLPrimitiveTypeTriangleStrip;
	}
	return MTLPrimitiveTypeTriangle;
}

static MTLWinding convert_front_face(kore_metal_front_face face) {
	return face == KORE_METAL_FRONT_FACE_CCW ? MTLWindingCounterClockwise : MTLWindingClockwise;
}

static MTLCullMode convert_cull_mode(kore_metal_cull_mode mode) {
	switch (mode) {
	case KORE_METAL_CULL_MODE_NONE:
		return MTLCullModeNone;
	case KORE_METAL_CULL_MODE_FRONT:
		return MTLCullModeFront;
	case KORE_METAL_CULL_MODE_BACK:
		return MTLCullModeBack;
	}
	return MTLCullModeNone;
}

static MTLStencilOperation convert_stencil_op(kore_metal_stencil_operation op) {
	switch (op) {
	case KORE_METAL_STENCIL_OPERATION_KEEP:
		return MTLStencilOperationKeep;
	case KORE_METAL_STENCIL_OPERATION_ZERO:
		return MTLStencilOperationZero;
	case KORE_METAL_STENCIL_OPERATION_REPLACE:
		return MTLStencilOperationReplace;
	case KORE_METAL_STENCIL_OPERATION_INVERT:
		return MTLStencilOperationInvert;
	case KORE_METAL_STENCIL_OPERATION_INCREMENT_CLAMP:
		return MTLStencilOperationIncrementClamp;
	case KORE_METAL_STENCIL_OPERATION_DECREMENT_CLAMP:
		return MTLStencilOperationDecrementClamp;
	case KORE_METAL_STENCIL_OPERATION_INCREMENT_WRAP:
		return MTLStencilOperationIncrementWrap;
	case KORE_METAL_STENCIL_OPERATION_DECREMENT_WRAP:
		return MTLStencilOperationDecrementWrap;
	}
	return MTLStencilOperationKeep;
}

void kore_metal_render_pipeline_init(kore_metal_device *device, kore_metal_render_pipeline *pipe, const kore_metal_render_pipeline_parameters *parameters) {
	id<MTLLibrary> library = (__bridge id<MTLLibrary>)device->library;

	id vertex_function   = [library newFunctionWithName:[NSString stringWithCString:parameters->vertex.shader.function_name encoding:NSUTF8StringEncoding]];
	id fragment_function = [library newFunctionWithName:[NSString stringWithCString:parameters->fragment.shader.function_name encoding:NSUTF8StringEncoding]];

	MTLRenderPipelineDescriptor *render_pipeline_descriptor = [[MTLRenderPipelineDescriptor alloc] init];
	render_pipeline_descriptor.vertexFunction               = vertex_function;
	render_pipeline_descriptor.fragmentFunction             = fragment_function;

	for (int i = 0; i < parameters->fragment.targets_count; ++i) {
		render_pipeline_descriptor.colorAttachments[i].pixelFormat = convert_format(parameters->fragment.targets[i].format);

		render_pipeline_descriptor.colorAttachments[i].blendingEnabled =
		    parameters->fragment.targets[i].blend.color.src_factor != KORE_METAL_BLEND_FACTOR_ONE ||
		    parameters->fragment.targets[i].blend.color.dst_factor != KORE_METAL_BLEND_FACTOR_ZERO ||
		    parameters->fragment.targets[i].blend.alpha.src_factor != KORE_METAL_BLEND_FACTOR_ONE ||
		    parameters->fragment.targets[i].blend.alpha.dst_factor != KORE_METAL_BLEND_FACTOR_ZERO;

		render_pipeline_descriptor.colorAttachments[i].sourceRGBBlendFactor = convert_blending_factor(parameters->fragment.targets[i].blend.color.src_factor);
		render_pipeline_descriptor.colorAttachments[i].destinationRGBBlendFactor =
		    convert_blending_factor(parameters->fragment.targets[i].blend.color.dst_factor);
		render_pipeline_descriptor.colorAttachments[i].rgbBlendOperation = convert_blending_operation(parameters->fragment.targets[i].blend.color.operation);

		render_pipeline_descriptor.colorAttachments[i].sourceAlphaBlendFactor = convert_blending_factor(parameters->fragment.targets[i].blend.alpha.src_factor);
		render_pipeline_descriptor.colorAttachments[i].destinationAlphaBlendFactor =
		    convert_blending_factor(parameters->fragment.targets[i].blend.alpha.dst_factor);
		render_pipeline_descriptor.colorAttachments[i].alphaBlendOperation = convert_blending_operation(parameters->fragment.targets[i].blend.alpha.operation);

		render_pipeline_descriptor.colorAttachments[i].writeMask = parameters->fragment.targets[i].write_mask;
	}

	if (has_depth(parameters->depth_stencil.format)) {
		render_pipeline_descriptor.depthAttachmentPixelFormat = convert_format(parameters->depth_stencil.format);
	}
	else {
		render_pipeline_descriptor.depthAttachmentPixelFormat = MTLPixelFormatInvalid;
	}

	if (has_stencil(parameters->depth_stencil.format)) {
		render_pipeline_descriptor.stencilAttachmentPixelFormat = convert_format(parameters->depth_stencil.format);
	}
	else {
		render_pipeline_descriptor.stencilAttachmentPixelFormat = MTLPixelFormatInvalid;
	}

	MTLVertexDescriptor *vertex_descriptor = [[MTLVertexDescriptor alloc] init];

	uint32_t bindings_count = 0;
	for (int i = 0; i < parameters->vertex.buffers_count; ++i) {
		++bindings_count;
	}

	uint32_t metal_attribute_index = 0;
	for (uint32_t binding_index = 0; binding_index < bindings_count; ++binding_index) {
		uint32_t offset = 0;

		for (uint32_t attribute_index = 0; attribute_index < parameters->vertex.buffers[binding_index].attributes_count; ++attribute_index) {
			kore_metal_vertex_attribute attribute = parameters->vertex.buffers[binding_index].attributes[attribute_index];

			vertex_descriptor.attributes[metal_attribute_index].bufferIndex = 0;
			vertex_descriptor.attributes[metal_attribute_index].offset      = offset;
			vertex_descriptor.attributes[metal_attribute_index].bufferIndex = binding_index;

			offset += vertex_attribute_size(attribute.format);

			switch (attribute.format) {
			case KORE_METAL_VERTEX_FORMAT_FLOAT32:
				vertex_descriptor.attributes[metal_attribute_index].format = MTLVertexFormatFloat;
				break;
			case KORE_METAL_VERTEX_FORMAT_FLOAT32X2:
				vertex_descriptor.attributes[metal_attribute_index].format = MTLVertexFormatFloat2;
				break;
			case KORE_METAL_VERTEX_FORMAT_FLOAT32X3:
				vertex_descriptor.attributes[metal_attribute_index].format = MTLVertexFormatFloat3;
				break;
			case KORE_METAL_VERTEX_FORMAT_FLOAT32X4:
				vertex_descriptor.attributes[metal_attribute_index].format = MTLVertexFormatFloat4;
				break;
			case KORE_METAL_VERTEX_FORMAT_SINT8X2:
				vertex_descriptor.attributes[metal_attribute_index].format = MTLVertexFormatChar2;
				break;
			case KORE_METAL_VERTEX_FORMAT_UINT8X2:
				vertex_descriptor.attributes[metal_attribute_index].format = MTLVertexFormatUChar2;
				break;
			case KORE_METAL_VERTEX_FORMAT_SNORM8X2:
				vertex_descriptor.attributes[metal_attribute_index].format = MTLVertexFormatChar2Normalized;
				break;
			case KORE_METAL_VERTEX_FORMAT_UNORM8X2:
				vertex_descriptor.attributes[metal_attribute_index].format = MTLVertexFormatUChar2Normalized;
				break;
			case KORE_METAL_VERTEX_FORMAT_SINT8X4:
				vertex_descriptor.attributes[metal_attribute_index].format = MTLVertexFormatChar4;
				break;
			case KORE_METAL_VERTEX_FORMAT_UINT8X4:
				vertex_descriptor.attributes[metal_attribute_index].format = MTLVertexFormatUChar4;
				break;
			case KORE_METAL_VERTEX_FORMAT_SNORM8X4:
				vertex_descriptor.attributes[metal_attribute_index].format = MTLVertexFormatChar4Normalized;
				break;
			case KORE_METAL_VERTEX_FORMAT_UNORM8X4:
				vertex_descriptor.attributes[metal_attribute_index].format = MTLVertexFormatUChar4Normalized;
				break;
			case KORE_METAL_VERTEX_FORMAT_SINT16X2:
				vertex_descriptor.attributes[metal_attribute_index].format = MTLVertexFormatShort2;
				break;
			case KORE_METAL_VERTEX_FORMAT_UINT16X2:
				vertex_descriptor.attributes[metal_attribute_index].format = MTLVertexFormatUShort2;
				break;
			case KORE_METAL_VERTEX_FORMAT_SNORM16X2:
				vertex_descriptor.attributes[metal_attribute_index].format = MTLVertexFormatShort2Normalized;
				break;
			case KORE_METAL_VERTEX_FORMAT_UNORM16X2:
				vertex_descriptor.attributes[metal_attribute_index].format = MTLVertexFormatUShort2Normalized;
				break;
			case KORE_METAL_VERTEX_FORMAT_SINT16X4:
				vertex_descriptor.attributes[metal_attribute_index].format = MTLVertexFormatShort4;
				break;
			case KORE_METAL_VERTEX_FORMAT_UINT16X4:
				vertex_descriptor.attributes[metal_attribute_index].format = MTLVertexFormatUShort4;
				break;
			case KORE_METAL_VERTEX_FORMAT_SNORM16X4:
				vertex_descriptor.attributes[metal_attribute_index].format = MTLVertexFormatShort4Normalized;
				break;
			case KORE_METAL_VERTEX_FORMAT_UNORM16X4:
				vertex_descriptor.attributes[metal_attribute_index].format = MTLVertexFormatUShort4Normalized;
				break;
			case KORE_METAL_VERTEX_FORMAT_SINT32:
				vertex_descriptor.attributes[metal_attribute_index].format = MTLVertexFormatInt;
				break;
			case KORE_METAL_VERTEX_FORMAT_UINT32:
				vertex_descriptor.attributes[metal_attribute_index].format = MTLVertexFormatUInt;
				break;
			case KORE_METAL_VERTEX_FORMAT_SINT32X2:
				vertex_descriptor.attributes[metal_attribute_index].format = MTLVertexFormatInt2;
				break;
			case KORE_METAL_VERTEX_FORMAT_UINT32X2:
				vertex_descriptor.attributes[metal_attribute_index].format = MTLVertexFormatUInt2;
				break;
			case KORE_METAL_VERTEX_FORMAT_SINT32X3:
				vertex_descriptor.attributes[metal_attribute_index].format = MTLVertexFormatInt3;
				break;
			case KORE_METAL_VERTEX_FORMAT_UINT32X3:
				vertex_descriptor.attributes[metal_attribute_index].format = MTLVertexFormatUInt3;
				break;
			case KORE_METAL_VERTEX_FORMAT_SINT32X4:
				vertex_descriptor.attributes[metal_attribute_index].format = MTLVertexFormatInt4;
				break;
			case KORE_METAL_VERTEX_FORMAT_UINT32X4:
				vertex_descriptor.attributes[metal_attribute_index].format = MTLVertexFormatUInt4;
				break;
			default:
				assert(false);
				break;
			}

			metal_attribute_index += 1;
		}

		vertex_descriptor.layouts[binding_index].stride       = offset;
		vertex_descriptor.layouts[binding_index].stepFunction = parameters->vertex.buffers[binding_index].step_mode == KORE_METAL_VERTEX_STEP_MODE_INSTANCE
		                                                            ? MTLVertexStepFunctionPerInstance
		                                                            : MTLVertexStepFunctionPerVertex;
	}

	render_pipeline_descriptor.vertexDescriptor = vertex_descriptor;

	NSError                     *errors       = nil;
	MTLRenderPipelineReflection *reflection   = nil;
	id<MTLDevice>                metal_device = (__bridge id<MTLDevice>)device->device;

	pipe->pipeline = (__bridge_retained void *)[metal_device newRenderPipelineStateWithDescriptor:render_pipeline_descriptor
	                                                                                      options:MTLPipelineOptionBufferTypeInfo
	                                                                                   reflection:&reflection
	                                                                                        error:&errors];

	pipe->depth_stencil_state = NULL;
	if (has_depth(parameters->depth_stencil.format) || has_stencil(parameters->depth_stencil.format)) {
		MTLDepthStencilDescriptor *depth_stencil_descriptor = [[MTLDepthStencilDescriptor alloc] init];

		depth_stencil_descriptor.depthCompareFunction = convert_compare(parameters->depth_stencil.depth_compare);
		depth_stencil_descriptor.depthWriteEnabled    = parameters->depth_stencil.depth_write_enabled;

		if (has_stencil(parameters->depth_stencil.format)) {
			MTLStencilDescriptor *front_stencil = [[MTLStencilDescriptor alloc] init];
			front_stencil.stencilCompareFunction = convert_compare(parameters->depth_stencil.stencil_front.compare);
			front_stencil.stencilFailureOperation = convert_stencil_op(parameters->depth_stencil.stencil_front.fail_op);
			front_stencil.depthFailureOperation = convert_stencil_op(parameters->depth_stencil.stencil_front.depth_fail_op);
			front_stencil.depthStencilPassOperation = convert_stencil_op(parameters->depth_stencil.stencil_front.pass_op);
			front_stencil.readMask = parameters->depth_stencil.stencil_read_mask;
			front_stencil.writeMask = parameters->depth_stencil.stencil_write_mask;
			depth_stencil_descriptor.frontFaceStencil = front_stencil;

			MTLStencilDescriptor *back_stencil = [[MTLStencilDescriptor alloc] init];
			back_stencil.stencilCompareFunction = convert_compare(parameters->depth_stencil.stencil_back.compare);
			back_stencil.stencilFailureOperation = convert_stencil_op(parameters->depth_stencil.stencil_back.fail_op);
			back_stencil.depthFailureOperation = convert_stencil_op(parameters->depth_stencil.stencil_back.depth_fail_op);
			back_stencil.depthStencilPassOperation = convert_stencil_op(parameters->depth_stencil.stencil_back.pass_op);
			back_stencil.readMask = parameters->depth_stencil.stencil_read_mask;
			back_stencil.writeMask = parameters->depth_stencil.stencil_write_mask;
			depth_stencil_descriptor.backFaceStencil = back_stencil;
		}

		pipe->depth_stencil_state = (__bridge_retained void *)[metal_device newDepthStencilStateWithDescriptor:depth_stencil_descriptor];
	}

	pipe->primitive_type = convert_primitive_topology(parameters->primitive.topology);
	pipe->cull_mode = convert_cull_mode(parameters->primitive.cull_mode);
	pipe->front_face_winding = convert_front_face(parameters->primitive.front_face);
}

void kore_metal_render_pipeline_destroy(kore_metal_render_pipeline *pipe) {
	if (pipe->pipeline != NULL) {
		CFRelease(pipe->pipeline);
		pipe->pipeline = NULL;
	}
	if (pipe->depth_stencil_state != NULL) {
		CFRelease(pipe->depth_stencil_state);
		pipe->depth_stencil_state = NULL;
	}
}

void kore_metal_compute_pipeline_init(kore_metal_device *device, kore_metal_compute_pipeline *pipe, const kore_metal_compute_pipeline_parameters *parameters) {
	id<MTLLibrary> library = (__bridge id<MTLLibrary>)device->library;

	id compute_function = [library newFunctionWithName:[NSString stringWithCString:parameters->shader.function_name encoding:NSUTF8StringEncoding]];

	NSError      *errors       = nil;
	id<MTLDevice> metal_device = (__bridge id<MTLDevice>)device->device;

	pipe->pipeline = (__bridge_retained void *)[metal_device newComputePipelineStateWithFunction:compute_function error:&errors];
}

void kore_metal_compute_pipeline_destroy(kore_metal_compute_pipeline *pipe) {
	if (pipe->pipeline != NULL) {
		CFRelease(pipe->pipeline);
		pipe->pipeline = NULL;
	}
}

void kore_metal_ray_pipeline_init(kore_gpu_device *device, kore_metal_ray_pipeline *pipe, const kore_metal_ray_pipeline_parameters *parameters) {
#if __has_include(<Metal/MTLRayTracing.h>)
	MTL_DEVICE_GET(metal_device, device);
	MTL_LIBRARY_GET(library, device);

	MTLRayTracingPipelineDescriptor *descriptor = [[MTLRayTracingPipelineDescriptor alloc] init];

	if (parameters->gen_shader.function_name != NULL) {
		id<MTLFunction> ray_gen_func = [library newFunctionWithName:[NSString stringWithCString:parameters->gen_shader.function_name encoding:NSUTF8StringEncoding]];
		if (ray_gen_func != nil) {
			[descriptor setRayGenFunction:ray_gen_func];
		}
	}

	if (parameters->miss_shader.function_name != NULL) {
		id<MTLFunction> miss_func = [library newFunctionWithName:[NSString stringWithCString:parameters->miss_shader.function_name encoding:NSUTF8StringEncoding]];
		if (miss_func != nil) {
			[descriptor setMissFunction:miss_func];
		}
	}

	if (parameters->closest_shader.function_name != NULL || parameters->any_shader.function_name != NULL || parameters->intersection_shader.function_name != NULL) {
		MTLHitGroupDescriptor *hit_group = [[MTLHitGroupDescriptor alloc] init];

		if (parameters->closest_shader.function_name != NULL) {
			id<MTLFunction> closest_hit_func = [library newFunctionWithName:[NSString stringWithCString:parameters->closest_shader.function_name encoding:NSUTF8StringEncoding]];
			if (closest_hit_func != nil) {
				[hit_group setClosestHitFunction:closest_hit_func];
			}
		}

		if (parameters->any_shader.function_name != NULL) {
			id<MTLFunction> any_hit_func = [library newFunctionWithName:[NSString stringWithCString:parameters->any_shader.function_name encoding:NSUTF8StringEncoding]];
			if (any_hit_func != nil) {
				[hit_group setAnyHitFunction:any_hit_func];
			}
		}

		if (parameters->intersection_shader.function_name != NULL) {
			id<MTLFunction> intersection_func = [library newFunctionWithName:[NSString stringWithCString:parameters->intersection_shader.function_name encoding:NSUTF8StringEncoding]];
			if (intersection_func != nil) {
				[hit_group setIntersectionFunction:intersection_func];
			}
		}

		[descriptor setHitGroup:hit_group atIndex:0];
	}

	NSError *error = nil;
	id<MTLRayTracingPipelineState> pipeline = [metal_device newRayTracingPipelineStateWithDescriptor:descriptor error:&error];
	if (pipeline != nil && error == nil) {
		pipe->ray_gen_pipeline = (__bridge_retained void *)pipeline;
	}

	pipe->shader_binding_table = NULL;
#else
	(void)device;
	(void)parameters;
	pipe->ray_gen_pipeline = NULL;
	pipe->miss_pipeline = NULL;
	pipe->closest_hit_pipeline = NULL;
	pipe->intersection_pipeline = NULL;
	pipe->any_hit_pipeline = NULL;
	pipe->shader_binding_table = NULL;
#endif
}

void kore_metal_ray_pipeline_destroy(kore_metal_ray_pipeline *pipe) {
	if (pipe->ray_gen_pipeline != NULL) {
		CFRelease(pipe->ray_gen_pipeline);
		pipe->ray_gen_pipeline = NULL;
	}
	if (pipe->miss_pipeline != NULL) {
		CFRelease(pipe->miss_pipeline);
		pipe->miss_pipeline = NULL;
	}
	if (pipe->closest_hit_pipeline != NULL) {
		CFRelease(pipe->closest_hit_pipeline);
		pipe->closest_hit_pipeline = NULL;
	}
	if (pipe->intersection_pipeline != NULL) {
		CFRelease(pipe->intersection_pipeline);
		pipe->intersection_pipeline = NULL;
	}
	if (pipe->any_hit_pipeline != NULL) {
		CFRelease(pipe->any_hit_pipeline);
		pipe->any_hit_pipeline = NULL;
	}
	if (pipe->shader_binding_table != NULL) {
		CFRelease(pipe->shader_binding_table);
		pipe->shader_binding_table = NULL;
	}
}
