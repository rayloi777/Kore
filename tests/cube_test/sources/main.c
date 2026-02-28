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
static float time = 0.0f;

typedef struct {
    float x, y, z;
    float r, g, b;
} Vertex;

static Vertex vertices[24] = {
    // Front face (z=+1) - Red
    { -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f },
    { -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f },
    {  0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f },
    {  0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f },

    // Back face (z=-1) - Green
    {  0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f },
    {  0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f },
    { -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f },
    { -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f },

    // Top face (y=+1) - Blue (CCW from outside)
    { -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f },
    {  0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f },
    {  0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f },
    { -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f },

    // Bottom face (y=-1) - Yellow
    { -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f },
    {  0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f },
    {  0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f },
    { -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f },

    // Right face (x=+1) - Purple
    {  0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f },
    {  0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f },
    {  0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 1.0f },
    {  0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f },

    // Left face (x=-1) - Cyan
    { -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f },
    { -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f },
    { -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f },
    { -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f },
};

static uint16_t indices[36] = {
    // Front (CCW from outside)
    0, 2, 1,  0, 3, 2,
    // Back (CCW from outside)
    4, 6, 5,  4, 7, 6,
    // Top (CCW from outside)
    8, 10, 9,  8, 11, 10,
    // Bottom (CCW from outside)
    12, 14, 13,  12, 15, 14,
    // Right (CCW from outside)
    16, 18, 17,  16, 19, 18,
    // Left (CCW from outside)
    20, 22, 21,  20, 23, 22
};

static void update(void *data) {
    time += 0.01f;
    
    float fov = 3.14159f / 4.0f;
    float aspect = (float)width / (float)height;
    float near = 0.1f;
    float far = 100.0f;
    
    kore_matrix4x4 proj = kore_matrix4x4_perspective(fov, aspect, near, far);
    kore_matrix4x4 view = kore_matrix4x4_look_at((kore_float3){0, 0, 3}, (kore_float3){0, 0, 0}, (kore_float3){0, 1, 0});
    kore_matrix4x4 model_y = kore_matrix4x4_rotation_y(time);
    kore_matrix4x4 model_x = kore_matrix4x4_rotation_x(time * 0.7f);
    kore_matrix4x4 model = kore_matrix4x4_multiply(&model_y, &model_x);
    kore_matrix4x4 view_model = kore_matrix4x4_multiply(&view, &model);
    kore_matrix4x4 mvp = kore_matrix4x4_multiply(&proj, &view_model);
    
    constants_type uniforms = {0};
    for (int i = 0; i < 16; i++) {
        uniforms.mvp.m[i] = mvp.m[i];
    }
    
    {
        constants_type *ptr = constants_type_buffer_lock(&uniform_buffer, 0, 1);
        *ptr = uniforms;
        constants_type_buffer_unlock(&uniform_buffer);
    }
    
    kore_gpu_texture *framebuffer = kore_gpu_device_get_framebuffer(&device);

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

    // Use Kong's pipeline
    kong_set_render_pipeline_pipeline(&list);

    // Set vertex buffer using Kong helper
    kong_set_vertex_buffer_vertex_in(&list, &vertex_buffer);

    // Set uniforms via descriptor set
    kong_set_descriptor_set_everything(&list, &uniform_set);

    kore_gpu_command_list_set_index_buffer(&list, &index_buffer, KORE_GPU_INDEX_FORMAT_UINT16, 0);

    kore_gpu_command_list_draw_indexed(&list, 36, 1, 0, 0, 0);

    kore_gpu_command_list_end_render_pass(&list);

    kore_gpu_command_list_present(&list);

    kore_gpu_device_execute_command_list(&device, &list);
}

int kickstart(int argc, char **argv) {
    kore_init("Cube Test", width, height, NULL, NULL);
    kore_set_update_callback(update, NULL);

    kore_gpu_device_wishlist wishlist = {0};
    kore_gpu_device_create(&device, &wishlist);

    kong_init(&device);

    kore_gpu_device_create_command_list(&device, KORE_GPU_COMMAND_LIST_TYPE_GRAPHICS, &list);

    kong_create_buffer_vertex_in(&device, 24, &vertex_buffer);
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
