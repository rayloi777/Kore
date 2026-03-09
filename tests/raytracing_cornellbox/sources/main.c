#include <kore3/gpu/device.h>
#include <kore3/gpu/pipeline.h>
#include <kore3/gpu/sampler.h>
#include <kore3/math/matrix.h>
#include <kore3/system.h>
#include <kore3/window.h>
#include <kong.h>

#include <stdlib.h>
#include <stdbool.h>

static const int width = 800;
static const int height = 600;

static kore_gpu_device device;
static kore_gpu_command_list list;
static kore_gpu_buffer constants;
static everything_set everything;
static vertex_in_buffer vertices;
static kore_gpu_buffer indices;
static bool depth_texture_created = false;
static kore_gpu_texture depth_texture;

static void resize(int w, int h, void *data) {
    if (depth_texture_created) {
        kore_gpu_texture_destroy(&depth_texture);
    }
    kore_gpu_texture_parameters depth_params = {
        .width = w,
        .height = h,
        .depth_or_array_layers = 1,
        .mip_level_count = 1,
        .sample_count = 1,
        .dimension = KORE_GPU_TEXTURE_DIMENSION_2D,
        .format = KORE_GPU_TEXTURE_FORMAT_DEPTH32_FLOAT,
        .usage = KORE_GPU_TEXTURE_USAGE_RENDER_ATTACHMENT,
    };
    kore_gpu_device_create_texture(&device, &depth_params, &depth_texture);
    depth_texture_created = true;
}

static void update(void *data) {
    constants_type *constants_data = constants_type_buffer_lock(&constants, 0, 1);
    constants_data->mvp = kore_matrix4x4_identity();
    constants_type_buffer_unlock(&constants);
    
    kore_gpu_texture *framebuffer = kore_gpu_device_get_framebuffer(&device);
    
    kore_gpu_render_pass_parameters parameters = {0};
    parameters.color_attachments_count = 1;
    parameters.color_attachments[0].load_op = KORE_GPU_LOAD_OP_CLEAR;
    kore_gpu_color clear_color = {0.0f, 0.0f, 0.0f, 1.0f};
    parameters.color_attachments[0].clear_value = clear_color;
    parameters.color_attachments[0].texture.texture = framebuffer;
    parameters.color_attachments[0].texture.array_layer_count = 1;
    parameters.color_attachments[0].texture.mip_level_count = 1;
    parameters.color_attachments[0].texture.format = kore_gpu_device_framebuffer_format(&device);
    parameters.color_attachments[0].texture.dimension = KORE_GPU_TEXTURE_VIEW_DIMENSION_2D;
    parameters.depth_stencil_attachment.texture = &depth_texture;
    parameters.depth_stencil_attachment.depth_load_op = KORE_GPU_LOAD_OP_CLEAR;
    parameters.depth_stencil_attachment.depth_clear_value = 1.0f;
    parameters.depth_stencil_attachment.depth_store_op = KORE_GPU_STORE_OP_STORE;
    parameters.depth_stencil_attachment.depth_read_only = false;
    
    kore_gpu_command_list_begin_render_pass(&list, &parameters);
    
    kong_set_render_pipeline_pipeline(&list);
    kong_set_vertex_buffer_vertex_in(&list, &vertices);
    kore_gpu_command_list_set_index_buffer(&list, &indices, KORE_GPU_INDEX_FORMAT_UINT16, 0);
    kong_set_descriptor_set_everything(&list, &everything);
    kore_gpu_command_list_draw_indexed(&list, 6, 1, 0, 0, 0);
    
    kore_gpu_command_list_end_render_pass(&list);
    kore_gpu_command_list_present(&list);
    kore_gpu_device_execute_command_list(&device, &list);
}

int kickstart(int argc, char **argv) {
    kore_init("Raytracing Cornell Box", width, height, NULL, NULL);
    kore_set_update_callback(update, NULL);
    
    kore_gpu_device_wishlist wishlist = {0};
    kore_gpu_device_create(&device, &wishlist);
    kong_init(&device);
    kore_gpu_device_create_command_list(&device, KORE_GPU_COMMAND_LIST_TYPE_GRAPHICS, &list);
    
    int win_width = kore_window_width(0);
    int win_height = kore_window_height(0);
    
    resize(win_width, win_height, NULL);
    
    kong_create_buffer_vertex_in(&device, 3, &vertices);
    {
        vertex_in *v = kong_vertex_in_buffer_lock(&vertices);
        v[0].pos.x = -1.0f; v[0].pos.y = -1.0f; v[0].pos.z = 0.5f;
        v[0].col.x = 1.0f; v[0].col.y = 0.0f; v[0].col.z = 0.0f;
        v[1].pos.x =  1.0f; v[1].pos.y = -1.0f; v[1].pos.z = 0.5f;
        v[1].col.x = 0.0f; v[1].col.y = 1.0f; v[1].col.z = 0.0f;
        v[2].pos.x =  0.0f; v[2].pos.y =  1.0f; v[2].pos.z = 0.5f;
        v[2].col.x = 0.0f; v[2].col.y = 0.0f; v[2].col.z = 1.0f;
        kong_vertex_in_buffer_unlock(&vertices);
    }
    
    kore_gpu_buffer_parameters index_params = {
        .size = 6 * sizeof(uint16_t),
        .usage_flags = KORE_GPU_BUFFER_USAGE_INDEX | KORE_GPU_BUFFER_USAGE_CPU_WRITE,
    };
    kore_gpu_device_create_buffer(&device, &index_params, &indices);
    {
        uint16_t *id = (uint16_t *)kore_gpu_buffer_lock_all(&indices);
        id[0] = 0; id[1] = 1;
        id[2] = 2;
        id[3] = 0;
        id[4] = 2;
        id[5] = 1;
        kore_gpu_buffer_unlock(&indices);
    }
    
    everything_parameters params = {
        .constants = &constants,
    };
    kong_create_everything_set(&device, &params, &everything);
    
    kore_start();
    return 0;
}
