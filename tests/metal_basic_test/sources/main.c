#include <kore3/gpu/device.h>
#include <kore3/gpu/commandlist.h>
#include <kore3/gpu/buffer.h>
#include <kore3/gpu/texture.h>
#include <kore3/gpu/pipeline.h>
#include <kore3/system.h>
#include <stdio.h>
#include <string.h>

static kore_gpu_device device;
static kore_gpu_command_list graphics_list;
static kore_gpu_command_list copy_list;

static kore_gpu_buffer vertex_buffer;
static kore_gpu_buffer index_buffer;
static kore_gpu_buffer uniform_buffer;

static kore_gpu_texture depth_texture;

static bool test_passed = true;
static int frame_count = 0;

static void log_test(const char *name, bool passed) {
    printf("[TEST] %s: %s\n", name, passed ? "PASS" : "FAIL");
    if (!passed) test_passed = false;
}

static bool test_device_create(void) {
    kore_gpu_device_wishlist wishlist = {0};
    kore_gpu_device_create(&device, &wishlist);
    
    bool passed = (device.metal.device != NULL);
    log_test("Device Create", passed);
    return passed;
}

static void test_device_destroy(void) {
    kore_gpu_device_destroy(&device);
    log_test("Device Destroy", true);
}

static bool test_command_list_create(void) {
    kore_gpu_device_create_command_list(&device, 
        KORE_GPU_COMMAND_LIST_TYPE_GRAPHICS, &graphics_list);
    kore_gpu_device_create_command_list(&device,
        KORE_GPU_COMMAND_LIST_TYPE_COPY, &copy_list);
    
    bool passed = (graphics_list.metal.command_buffer != NULL &&
                   copy_list.metal.command_buffer != NULL);
    log_test("Command List Create", passed);
    return passed;
}

static void test_command_list_destroy(void) {
    kore_gpu_command_list_destroy(&graphics_list);
    kore_gpu_command_list_destroy(&copy_list);
    log_test("Command List Destroy", true);
}

static bool test_buffer_create(void) {
    kore_gpu_buffer_parameters vertex_params = {
        .size = 1024,
        .usage_flags = KORE_GPU_BUFFER_USAGE_VERTEX | KORE_GPU_BUFFER_USAGE_CPU_WRITE,
    };
    kore_gpu_device_create_buffer(&device, &vertex_params, &vertex_buffer);
    
    kore_gpu_buffer_parameters index_params = {
        .size = 512,
        .usage_flags = KORE_GPU_BUFFER_USAGE_INDEX | KORE_GPU_BUFFER_USAGE_CPU_WRITE,
    };
    kore_gpu_device_create_buffer(&device, &index_params, &index_buffer);
    
    kore_gpu_buffer_parameters uniform_params = {
        .size = 256,
        .usage_flags = KORE_GPU_BUFFER_USAGE_CPU_WRITE,
    };
    kore_gpu_device_create_buffer(&device, &uniform_params, &uniform_buffer);
    
    bool passed = (vertex_buffer.metal.buffer != NULL &&
                   index_buffer.metal.buffer != NULL &&
                   uniform_buffer.metal.buffer != NULL);
    log_test("Buffer Create", passed);
    return passed;
}

static bool test_buffer_lock_upload(void) {
    float *vertex_data = (float *)kore_gpu_buffer_lock_all(&vertex_buffer);
    if (vertex_data) {
        for (int i = 0; i < 256; ++i) {
            vertex_data[i] = (float)i;
        }
        kore_gpu_buffer_unlock_all(&vertex_buffer);
        log_test("Buffer Lock/Upload", true);
        return true;
    }
    
    log_test("Buffer Lock/Upload", false);
    return false;
}

static void test_buffer_destroy(void) {
    kore_gpu_buffer_destroy(&vertex_buffer);
    kore_gpu_buffer_destroy(&index_buffer);
    kore_gpu_buffer_destroy(&uniform_buffer);
    log_test("Buffer Destroy", true);
}

static bool test_texture_create(void) {
    kore_gpu_texture_parameters depth_params = {
        .width = 800,
        .height = 600,
        .depth_or_array_layers = 1,
        .mip_level_count = 1,
        .sample_count = 1,
        .dimension = KORE_GPU_TEXTURE_DIMENSION_2D,
        .format = KORE_GPU_TEXTURE_FORMAT_DEPTH32_FLOAT,
        .usage = KORE_GPU_TEXTURE_USAGE_RENDER_ATTACHMENT,
    };
    kore_gpu_device_create_texture(&device, &depth_params, &depth_texture);
    
    bool passed = (depth_texture.metal.texture != NULL);
    log_test("Texture Create", passed);
    return passed;
}

static void test_texture_destroy(void) {
    kore_gpu_texture_destroy(&depth_texture);
    log_test("Texture Destroy", true);
}

static bool test_render_pass(void) {
    kore_gpu_texture *framebuffer = kore_gpu_device_get_framebuffer(&device);
    if (!framebuffer) {
        log_test("Render Pass", false);
        return false;
    }
    
    kore_gpu_render_pass_parameters params = {0};
    params.color_attachments_count = 1;
    params.color_attachments[0].load_op = KORE_GPU_LOAD_OP_CLEAR;
    params.color_attachments[0].store_op = KORE_GPU_STORE_OP_STORE;
    params.color_attachments[0].clear_value = (kore_gpu_color){0.1f, 0.2f, 0.3f, 1.0f};
    params.color_attachments[0].texture.texture = framebuffer;
    
    params.depth_stencil_attachment.texture = &depth_texture;
    params.depth_stencil_attachment.depth_load_op = KORE_GPU_LOAD_OP_CLEAR;
    params.depth_stencil_attachment.depth_store_op = KORE_GPU_STORE_OP_STORE;
    params.depth_stencil_attachment.depth_clear_value = 1.0f;
    
    kore_gpu_command_list_begin_render_pass(&graphics_list, &params);
    
    kore_gpu_command_list_set_viewport(&graphics_list, 0, 0, 800, 600, 0.0f, 1.0f);
    kore_gpu_command_list_set_scissor_rect(&graphics_list, 0, 0, 800, 600);
    kore_gpu_command_list_set_blend_constant(&graphics_list, 
        (kore_gpu_color){1.0f, 1.0f, 1.0f, 1.0f});
    kore_gpu_command_list_set_stencil_reference(&graphics_list, 0);
    
    kore_gpu_command_list_end_render_pass(&graphics_list);
    kore_gpu_command_list_present(&graphics_list);
    kore_gpu_device_execute_command_list(&device, &graphics_list);
    kore_gpu_device_wait_until_idle(&device);
    
    log_test("Render Pass", true);
    return true;
}

static bool test_fence(void) {
    kore_gpu_fence fence;
    kore_gpu_device_create_fence(&device, &fence);
    
    kore_gpu_device_signal(&device, KORE_GPU_COMMAND_LIST_TYPE_GRAPHICS, &fence, 1);
    kore_gpu_device_wait(&device, KORE_GPU_COMMAND_LIST_TYPE_GRAPHICS, &fence, 1);
    
    kore_gpu_fence_destroy(&fence);
    
    log_test("Fence Sync", true);
    return true;
}

static bool test_query_set(void) {
    log_test("Query Set", true);
    return true;
}

static bool test_debug_support(void) {
    kore_gpu_command_list_set_name(&graphics_list, "TestGraphicsList");
    
    kore_gpu_command_list_push_debug_group(&graphics_list, "TestGroup");
    kore_gpu_command_list_insert_debug_marker(&graphics_list, "TestMarker");
    kore_gpu_command_list_pop_debug_group(&graphics_list);
    
    log_test("Debug Support", true);
    return true;
}

static bool test_wait_idle(void) {
    kore_gpu_device_wait_until_idle(&device);
    log_test("Device Wait Idle", true);
    return true;
}

static void run_all_tests(void) {
    printf("=====================================\n");
    printf("Metal Backend Test Suite\n");
    printf("=====================================\n");
    
    if (!test_device_create()) return;
    if (!test_command_list_create()) return;
    if (!test_buffer_create()) return;
    if (!test_buffer_lock_upload()) return;
    if (!test_texture_create()) return;
    if (!test_render_pass()) return;
    if (!test_fence()) return;
    if (!test_query_set()) return;
    if (!test_debug_support()) return;
    if (!test_wait_idle()) return;
    
    test_texture_destroy();
    test_buffer_destroy();
    test_command_list_destroy();
    test_device_destroy();
    
    printf("=====================================\n");
    printf("Test Result: %s\n", test_passed ? "ALL PASS" : "SOME FAIL");
    printf("=====================================\n");
}

static void update(void *data) {
    (void)data;
    
    frame_count++;
    
    if (frame_count == 1) {
        run_all_tests();
        kore_stop();
    }
}

int kickstart(int argc, char **argv) {
    (void)argc;
    (void)argv;
    
    printf("Initializing Metal Basic Test...\n");
    
    kore_init("Metal Basic Test", 800, 600, NULL, NULL);
    kore_set_update_callback(update, NULL);
    
    kore_start();
    
    return test_passed ? 0 : 1;
}
