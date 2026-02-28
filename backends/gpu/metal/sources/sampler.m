#include <kore3/metal/sampler_functions.h>

#include "metalunit.h"

#include <kore3/gpu/sampler.h>

void kore_metal_sampler_create(kore_gpu_device *device, const kore_gpu_sampler_parameters *parameters, kore_gpu_sampler *sampler) {
	id<MTLDevice> metal_device = (__bridge id<MTLDevice>)device->metal.device;

	MTLSamplerDescriptor *descriptor = [[MTLSamplerDescriptor alloc] init];

	switch (parameters->min_filter) {
		case KORE_GPU_FILTER_MODE_NEAREST:
			descriptor.minFilter = MTLSamplerMinMagFilterNearest;
			break;
		case KORE_GPU_FILTER_MODE_LINEAR:
			descriptor.minFilter = MTLSamplerMinMagFilterLinear;
			break;
	}

	switch (parameters->mag_filter) {
		case KORE_GPU_FILTER_MODE_NEAREST:
			descriptor.magFilter = MTLSamplerMinMagFilterNearest;
			break;
		case KORE_GPU_FILTER_MODE_LINEAR:
			descriptor.magFilter = MTLSamplerMinMagFilterLinear;
			break;
	}

	switch (parameters->mipmap_filter) {
		case KORE_GPU_MIPMAP_FILTER_MODE_NEAREST:
			descriptor.mipFilter = MTLSamplerMipFilterNearest;
			break;
		case KORE_GPU_MIPMAP_FILTER_MODE_LINEAR:
			descriptor.mipFilter = MTLSamplerMipFilterLinear;
			break;
	}

	switch (parameters->address_mode_u) {
		case KORE_GPU_ADDRESS_MODE_CLAMP_TO_EDGE:
			descriptor.sAddressMode = MTLSamplerAddressModeClampToEdge;
			break;
		case KORE_GPU_ADDRESS_MODE_REPEAT:
			descriptor.sAddressMode = MTLSamplerAddressModeRepeat;
			break;
		case KORE_GPU_ADDRESS_MODE_MIRROR_REPEAT:
			descriptor.sAddressMode = MTLSamplerAddressModeMirrorRepeat;
			break;
		default:
			descriptor.sAddressMode = MTLSamplerAddressModeClampToEdge;
			break;
	}

	switch (parameters->address_mode_v) {
		case KORE_GPU_ADDRESS_MODE_CLAMP_TO_EDGE:
			descriptor.tAddressMode = MTLSamplerAddressModeClampToEdge;
			break;
		case KORE_GPU_ADDRESS_MODE_REPEAT:
			descriptor.tAddressMode = MTLSamplerAddressModeRepeat;
			break;
		case KORE_GPU_ADDRESS_MODE_MIRROR_REPEAT:
			descriptor.tAddressMode = MTLSamplerAddressModeMirrorRepeat;
			break;
		default:
			descriptor.tAddressMode = MTLSamplerAddressModeClampToEdge;
			break;
	}

	switch (parameters->address_mode_w) {
		case KORE_GPU_ADDRESS_MODE_CLAMP_TO_EDGE:
			descriptor.rAddressMode = MTLSamplerAddressModeClampToEdge;
			break;
		case KORE_GPU_ADDRESS_MODE_REPEAT:
			descriptor.rAddressMode = MTLSamplerAddressModeRepeat;
			break;
		case KORE_GPU_ADDRESS_MODE_MIRROR_REPEAT:
			descriptor.rAddressMode = MTLSamplerAddressModeMirrorRepeat;
			break;
		default:
			descriptor.rAddressMode = MTLSamplerAddressModeClampToEdge;
			break;
	}

	descriptor.lodMinClamp = parameters->lod_min_clamp;
	descriptor.lodMaxClamp = parameters->lod_max_clamp;

	if (parameters->max_anisotropy > 1) {
		descriptor.maxAnisotropy = parameters->max_anisotropy;
	}

	if (parameters->compare != KORE_GPU_COMPARE_FUNCTION_UNDEFINED) {
		descriptor.compareFunction = convert_compare(parameters->compare);
	}

	id<MTLSamplerState> metal_sampler = [metal_device newSamplerStateWithDescriptor:descriptor];
	sampler->metal.sampler = (__bridge_retained void *)metal_sampler;
}

void kore_metal_sampler_set_name(kore_gpu_sampler *sampler, const char *name) {}

void kore_metal_sampler_destroy(kore_gpu_sampler *sampler) {}

void kore_metal_device_create_default_sampler(kore_gpu_device *device, kore_gpu_sampler *sampler) {
	kore_gpu_sampler_parameters params = {
		.address_mode_u = KORE_GPU_ADDRESS_MODE_REPEAT,
		.address_mode_v = KORE_GPU_ADDRESS_MODE_REPEAT,
		.address_mode_w = KORE_GPU_ADDRESS_MODE_REPEAT,
		.min_filter = KORE_GPU_FILTER_MODE_LINEAR,
		.mag_filter = KORE_GPU_FILTER_MODE_LINEAR,
		.mipmap_filter = KORE_GPU_MIPMAP_FILTER_MODE_LINEAR,
		.lod_min_clamp = 0.0f,
		.lod_max_clamp = 1000.0f,
		.compare = KORE_GPU_COMPARE_FUNCTION_ALWAYS,
		.max_anisotropy = 16,
	};
	kore_metal_sampler_create(device, &params, sampler);
}
