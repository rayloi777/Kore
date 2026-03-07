#include <kore3/gpu/device.h>
#include <kore3/gpu/pipeline.h>
#include <kore3/io/filereader.h>
#include <kore3/math/matrix.h>
#include <kore3/system.h>

#include <kong.h>

#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

static kore_gpu_device       device;
static kore_gpu_command_list list;
static vertex_in_buffer      vertex_buffer;
static kore_gpu_buffer       index_buffer;
static kore_gpu_buffer       uniform_buffer;
static everything_set        uniform_set;
static kore_gpu_texture      depth_texture;

static const int width  = 1920;
static const int height = 1080;

static vertex_in vertices[3] = {
    { {  0.0f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
    { { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
    { {  0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
};

static uint16_t indices[3] = {
    0, 1, 2
};

static void update(void *data) {
    kore_gpu_texture *framebuffer = kore_gpu_device_get_framebuffer(&device);
    float fb_width = (float)framebuffer->width;
    float fb_height = (float)framebuffer->height;
    float aspect = fb_width / fb_height;
    float fov = 3.14159f / 4.0f;
    
    kore_float3 eye = {0, 0, 4};
    kore_float3 center = {0, 0, 0};
    kore_float3 up = {0, 1, 0};
    
    kore_matrix4x4 projection = kore_matrix4x4_perspective(fov, aspect, 0.1f, 100.0f);
    kore_matrix4x4 view = kore_matrix4x4_look_at(eye, center, up);
    kore_matrix4x4 model = kore_matrix4x4_identity();
    
    kore_matrix4x4 mvp = kore_matrix4x4_identity();
    mvp = kore_matrix4x4_multiply(&mvp, &projection);
    mvp = kore_matrix4x4_multiply(&mvp, &view);
    mvp = kore_matrix4x4_multiply(&mvp, &model);

    //kore_matrix4x4 mvp = kore_matrix4x4_translation(0.5f, 0, 0.5f); 
    
    constants_type uniforms = {0};
    for (int i = 0; i < 16; i++) {
        uniforms.mvp.m[i] = mvp.m[i];
    }
    
    {
        constants_type *ptr = constants_type_buffer_lock(&uniform_buffer, 0, 1);
        *ptr = uniforms;
        constants_type_buffer_unlock(&uniform_buffer);
    }
    
    kore_gpu_color clear_color = {
        .r = 0.1f,
        .g = 0.1f,
        .b = 0.2f,
        .a = 1.0f,
    };

    kore_gpu_render_pass_parameters parameters = {
        .color_attachments_count = 1,
        .color_attachments = {
            {
                .load_op = KORE_GPU_LOAD_OP_CLEAR,
                .clear_value = clear_color,
                .texture = {
                    .texture = framebuffer,
                    .array_layer_count = 1,
                    .mip_level_count = 1,
                    .format = KORE_GPU_TEXTURE_FORMAT_BGRA8_UNORM,
                    .dimension = KORE_GPU_TEXTURE_VIEW_DIMENSION_2D,
                },
            },
        },
        .depth_stencil_attachment = {
            .texture = &depth_texture,
            .depth_load_op = KORE_GPU_LOAD_OP_CLEAR,
            .depth_store_op = KORE_GPU_STORE_OP_STORE,
            .depth_clear_value = 1.0f,
            .stencil_load_op = KORE_GPU_LOAD_OP_LOAD,
            .stencil_store_op = KORE_GPU_STORE_OP_DISCARD,
            .stencil_clear_value = 0,
        },
    };
    kore_gpu_command_list_begin_render_pass(&list, &parameters);

    kong_set_render_pipeline_pipeline(&list);

    kong_set_vertex_buffer_vertex_in(&list, &vertex_buffer);

    kong_set_descriptor_set_everything(&list, &uniform_set);

    kore_gpu_command_list_set_index_buffer(&list, &index_buffer, KORE_GPU_INDEX_FORMAT_UINT16, 0);

    kore_gpu_command_list_draw_indexed(&list, 3, 1, 0, 0, 0);

    kore_gpu_command_list_end_render_pass(&list);

    kore_gpu_command_list_present(&list);

    kore_gpu_device_execute_command_list(&device, &list);
}

int kickstart(int argc, char **argv) {
    kore_init("Triangle Test", width, height, NULL, NULL);
    kore_set_update_callback(update, NULL);

    kore_gpu_device_wishlist wishlist = {0};
    kore_gpu_device_create(&device, &wishlist);

    kong_init(&device);

    kore_gpu_device_create_command_list(&device, KORE_GPU_COMMAND_LIST_TYPE_GRAPHICS, &list);

    kong_create_buffer_vertex_in(&device, 3, &vertex_buffer);
    {
        vertex_in *ptr = kong_vertex_in_buffer_lock(&vertex_buffer);
        memcpy(ptr, vertices, sizeof(vertices));
        kong_vertex_in_buffer_unlock(&vertex_buffer);
    }

    kore_gpu_buffer_parameters index_params = {
        .size = sizeof(indices),
        .usage_flags = KORE_GPU_BUFFER_USAGE_INDEX | KORE_GPU_BUFFER_USAGE_COPY_DST,
    };
    kore_gpu_device_create_buffer(&device, &index_params, &index_buffer);
    kore_gpu_buffer_upload(&device, indices, sizeof(indices), index_params.usage_flags, &index_buffer);

    constants_type_buffer_create(&device, &uniform_buffer, 1);

    kore_gpu_texture_parameters depth_params = {
        .width = width,
        .height = height,
        .depth_or_array_layers = 1,
        .mip_level_count = 1,
        .sample_count = 1,
        .dimension = KORE_GPU_TEXTURE_DIMENSION_2D,
        .format = KORE_GPU_TEXTURE_FORMAT_DEPTH32_FLOAT,
        .usage = KORE_GPU_TEXTURE_USAGE_RENDER_ATTACHMENT,
    };
    kore_gpu_device_create_texture(&device, &depth_params, &depth_texture);

    everything_parameters params = {
        .constants = &uniform_buffer,
    };
    kong_create_everything_set(&device, &params, &uniform_set);

    kore_start();

    return 0;
}
