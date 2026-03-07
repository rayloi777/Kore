#include <kore3/gpu/device.h>
#include <kore3/gpu/pipeline.h>
#include <kore3/gpu/sampler.h>
#include <kore3/image.h>
#include <kore3/io/filereader.h>
#include <kore3/math/matrix.h>
#include <kore3/system.h>

#include <kong.h>

#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

static const int width  = 1920;
static const int height = 1080;

typedef struct {
    float x, y, z;
    float u, v;
} Vertex;

static kore_gpu_device       device;
static kore_gpu_command_list list;
static vertex_in_buffer     vertex_buffer;
static kore_gpu_buffer       index_buffer;
static kore_gpu_buffer       uniform_buffer;
static mvp_set               uniform_set;
static kore_gpu_texture      depth_texture;
static kore_gpu_texture      test_texture;
static kore_gpu_texture_view test_texture_view;
static kore_gpu_sampler     test_sampler;
static textures_set         texture_set;

static Vertex vertices[4] = {
    { -0.5f, -0.5f, 0.0f, 0.0f, 0.0f },
    {  0.5f, -0.5f, 0.0f, 1.0f, 0.0f },
    {  0.5f,  0.5f, 0.0f, 1.0f, 1.0f },
    { -0.5f,  0.5f, 0.0f, 0.0f, 1.0f },
};

static uint16_t indices[6] = {
    0, 1, 2,
    0, 2, 3,
};

static void update(void *data) {
    kore_matrix4x4 mvp = kore_matrix4x4_identity();
    
    constants_type uniforms = {
        .mvp = mvp
    };
    
    {
        constants_type *ptr = constants_type_buffer_lock(&uniform_buffer, 0, 1);
        if (ptr) {
            *ptr = uniforms;
            constants_type_buffer_unlock(&uniform_buffer);
        }
    }
    
    kore_gpu_texture *framebuffer = kore_gpu_device_get_framebuffer(&device);

    kore_gpu_color clear_color = {
        .r = 0.2f,
        .g = 0.2f,
        .b = 0.3f,
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
    kong_set_descriptor_set_mvp(&list, &uniform_set);
    kong_set_descriptor_set_textures(&list, &texture_set);

    kore_gpu_command_list_set_index_buffer(&list, &index_buffer, KORE_GPU_INDEX_FORMAT_UINT16, 0);
    kore_gpu_command_list_draw_indexed(&list, 6, 1, 0, 0, 0);

    kore_gpu_command_list_end_render_pass(&list);
    kore_gpu_command_list_present(&list);
    kore_gpu_device_execute_command_list(&device, &list);
}

int kickstart(int argc, char **argv) {
    kore_init("Texture Test", width, height, NULL, NULL);
    kore_set_update_callback(update, NULL);

    kore_gpu_device_wishlist wishlist = {0};
    kore_gpu_device_create(&device, &wishlist);

    kong_init(&device);

    kore_gpu_device_create_command_list(&device, KORE_GPU_COMMAND_LIST_TYPE_GRAPHICS, &list);

    kore_gpu_buffer_parameters vertex_params = {
        .size = sizeof(vertices),
        .usage_flags = KORE_GPU_BUFFER_USAGE_VERTEX | KORE_GPU_BUFFER_USAGE_COPY_DST,
    };
    kore_gpu_device_create_buffer(&device, &vertex_params, &vertex_buffer.buffer);
    
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
    
    {
        void *ptr = kore_gpu_buffer_lock_all(&index_buffer);
        memcpy(ptr, indices, sizeof(indices));
        kore_gpu_buffer_unlock_all(&index_buffer);
    }

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

    kore_image image;
    uint8_t *image_memory = (uint8_t *)malloc(512 * 512 * 4);
    
    size_t image_size = kore_image_init_from_file(&image, image_memory, "haxe.png");
    
    if (image_size == 0) {
        for (int i = 0; i < 512 * 512; i++) {
            int checker = ((i % 512) / 32 + (i / 512) / 32) % 2;
            image_memory[i * 4 + 0] = checker ? 255 : 0;
            image_memory[i * 4 + 1] = checker ? 0 : 255;
            image_memory[i * 4 + 2] = 0;
            image_memory[i * 4 + 3] = 255;
        }
        image.width = 512;
        image.height = 512;
    }
    image.depth = 1;
    image.format = KORE_IMAGE_FORMAT_RGBA32;
    image.data = image_memory;

    kore_gpu_texture_parameters tex_params = {
        .width = image.width,
        .height = image.height,
        .depth_or_array_layers = 1,
        .mip_level_count = 1,
        .sample_count = 1,
        .dimension = KORE_GPU_TEXTURE_DIMENSION_2D,
        .format = KORE_GPU_TEXTURE_FORMAT_RGBA8_UNORM,
        .usage = KORE_GPU_TEXTURE_USAGE_SAMPLED | KORE_GPU_TEXTURE_USAGE_COPY_DST,
    };
    kore_gpu_device_create_texture(&device, &tex_params, &test_texture);

    kore_gpu_texture_upload(&device, &test_texture, kore_image_get_pixels(&image), image.width, image.height);

    kore_image_destroy(&image);
    free(image_memory);

    kore_gpu_texture_view_create(&device, &test_texture, &test_texture_view);

    kore_gpu_device_create_default_sampler(&device, &test_sampler);

    textures_parameters textures_params = {
        .tex = test_texture_view,
        .sam = &test_sampler,
    };
    kong_create_textures_set(&device, &textures_params, &texture_set);

    mvp_parameters mvp_params = {
        .constants = &uniform_buffer,
    };
    kong_create_mvp_set(&device, &mvp_params, &uniform_set);

    kore_start();

    return 0;
}
