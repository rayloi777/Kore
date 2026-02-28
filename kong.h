#ifndef KONG_INTEGRATION_HEADER
#define KONG_INTEGRATION_HEADER

#include <kore3/gpu/device.h>
#include <kore3/gpu/sampler.h>
#include <kore3/opengl/descriptorset_structs.h>
#include <kore3/opengl/pipeline_structs.h>
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



#ifdef __cplusplus
}
#endif

#endif
