#ifndef KORE_METAL_SAMPLER_FUNCTIONS_HEADER
#define KORE_METAL_SAMPLER_FUNCTIONS_HEADER

#include <kore3/gpu/sampler.h>

#ifdef __cplusplus
extern "C" {
#endif

void kore_metal_sampler_create(kore_gpu_device *device, const kore_gpu_sampler_parameters *parameters, kore_gpu_sampler *sampler);

void kore_metal_sampler_set_name(kore_gpu_sampler *sampler, const char *name);

void kore_metal_sampler_destroy(kore_gpu_sampler *sampler);

void kore_metal_device_create_default_sampler(kore_gpu_device *device, kore_gpu_sampler *sampler);

#ifdef __cplusplus
}
#endif

#endif
