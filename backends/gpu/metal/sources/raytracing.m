#include <kore3/metal/raytracing_functions.h>

void kore_metal_raytracing_volume_destroy(kore_gpu_raytracing_volume *volume) {
	if (volume->metal.acceleration_structure != NULL) {
		CFRelease(volume->metal.acceleration_structure);
		volume->metal.acceleration_structure = NULL;
	}
	if (volume->metal.scratch_buffer != NULL) {
		CFRelease(volume->metal.scratch_buffer);
		volume->metal.scratch_buffer = NULL;
	}
	if (volume->metal.compacted_size_buffer != NULL) {
		CFRelease(volume->metal.compacted_size_buffer);
		volume->metal.compacted_size_buffer = NULL;
	}
}

void kore_metal_raytracing_hierarchy_destroy(kore_gpu_raytracing_hierarchy *hierarchy) {
	if (hierarchy->metal.acceleration_structure != NULL) {
		CFRelease(hierarchy->metal.acceleration_structure);
		hierarchy->metal.acceleration_structure = NULL;
	}
	if (hierarchy->metal.instance_buffer != NULL) {
		CFRelease(hierarchy->metal.instance_buffer);
		hierarchy->metal.instance_buffer = NULL;
	}
	if (hierarchy->metal.scratch_buffer != NULL) {
		CFRelease(hierarchy->metal.scratch_buffer);
		hierarchy->metal.scratch_buffer = NULL;
	}
}
