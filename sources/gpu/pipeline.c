#include <kore3/gpu/pipeline.h>

#if defined(KORE_DIRECT3D11)
#include <kore3/direct3d11/pipeline_functions.h>
#elif defined(KORE_DIRECT3D12)
#include <kore3/direct3d12/pipeline_functions.h>
#elif defined(KORE_METAL)
#include <kore3/metal/pipeline_functions.h>
#elif defined(KORE_OPENGL)
#include <kore3/opengl/pipeline_functions.h>
#elif defined(KORE_VULKAN)
#include <kore3/vulkan/pipeline_functions.h>
#elif defined(KORE_WEBGPU)
#include <kore3/webgpu/pipeline_functions.h>
#elif defined(KORE_KOMPJUTA)
#include <kore3/kompjuta/pipeline_functions.h>
#elif defined(KORE_CONSOLE)
#include <kore3/console/pipeline_functions.h>
#endif

#include <assert.h>

void kore_gpu_render_pipeline_init(kore_gpu_device *device, kore_gpu_render_pipeline *pipeline, const kore_gpu_render_pipeline_parameters *parameters) {
	KORE_GPU_CALL3(render_pipeline_init, device, pipeline, parameters);
}

void kore_gpu_render_pipeline_destroy(kore_gpu_render_pipeline *pipeline) {
	KORE_GPU_CALL1(render_pipeline_destroy, pipeline);
}

void kore_gpu_compute_pipeline_init(kore_gpu_device *device, kore_gpu_compute_pipeline *pipeline, const kore_gpu_compute_pipeline_parameters *parameters) {
	KORE_GPU_CALL3(compute_pipeline_init, device, pipeline, parameters);
}

void kore_gpu_compute_pipeline_destroy(kore_gpu_compute_pipeline *pipeline) {
	KORE_GPU_CALL1(compute_pipeline_destroy, pipeline);
}

void kore_gpu_ray_pipeline_init(kore_gpu_device *device, kore_gpu_ray_pipeline *pipeline, const kore_gpu_ray_pipeline_parameters *parameters) {
	KORE_GPU_CALL3(ray_pipeline_init, device, pipeline, parameters);
}

void kore_gpu_ray_pipeline_destroy(kore_gpu_ray_pipeline *pipeline) {
	KORE_GPU_CALL1(ray_pipeline_destroy, pipeline);
}
