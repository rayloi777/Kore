#include "kong.h"


#include <kore3/opengl/buffer_functions.h>
#include <kore3/opengl/commandlist_functions.h>
#include <kore3/opengl/device_functions.h>
#include <kore3/opengl/descriptorset_functions.h>
#include <kore3/opengl/pipeline_functions.h>
#include <kore3/opengl/texture_functions.h>
#include <kore3/util/align.h>

#include <assert.h>
#include <stdlib.h>


uint32_t kore_opengl_find_uniform_block_index(unsigned program, const char *name);

void kong_init(kore_gpu_device *device) {
}
