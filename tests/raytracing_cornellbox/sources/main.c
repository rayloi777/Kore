#include <kore3/gpu/device.h>
#include <kore3/gpu/pipeline.h>
#include <kore3/gpu/sampler.h>
#include <kore3/input/keyboard.h>
#include <kore3/input/mouse.h>
#include <kore3/math/matrix.h>
#include <kore3/system.h>

#include <kong.h>

#include <stdlib.h>
#include <math.h>

static const int width  = 800;
static const int height = 600;

static kore_gpu_device       device;
static kore_gpu_command_list list;
static vertex_in_buffer      vertices;
static kore_gpu_buffer       indices;
static kore_gpu_buffer       constants;
static kore_gpu_buffer       rc_buffer;
static kore_gpu_buffer       vertex_buffer_storage;
static kore_gpu_buffer       index_buffer_storage;
static kore_gpu_texture      render_target_texture;
static kore_gpu_sampler      sampler;
static everything_set        everything;
static compute_set           compute;

static float camera_x = 0.0f;
static float camera_y = 0.0f;
static float camera_z = -2.0f;
static float camera_yaw = 0.0f;
static float camera_pitch = 0.0f;

static int key_w = 0;
static int key_a = 0;
static int key_s = 0;
static int key_d = 0;
static int key_q = 0;
static int key_e = 0;

static void key_down(int key, void *data) {
	if (key == 87) key_w = 1;
	if (key == 65) key_a = 1;
	if (key == 83) key_s = 1;
	if (key == 68) key_d = 1;
	if (key == 81) key_q = 1;
	if (key == 69) key_e = 1;
}

static void key_up(int key, void *data) {
	if (key == 87) key_w = 0;
	if (key == 65) key_a = 0;
	if (key == 83) key_s = 0;
	if (key == 68) key_d = 0;
	if (key == 81) key_q = 0;
	if (key == 69) key_e = 0;
}

static void mouse_move(int window, int x, int y, int movement_x, int movement_y, void *data) {
	camera_yaw += (float)movement_x * 0.005f;
	camera_pitch -= (float)movement_y * 0.005f;
	if (camera_pitch > 1.5f) camera_pitch = 1.5f;
	if (camera_pitch < -1.5f) camera_pitch = -1.5f;
}

static void update(void *data) {
	float move_speed = 0.05f;
	
	float cos_yaw = cosf(camera_yaw);
	float sin_yaw = sinf(camera_yaw);
	
	if (key_w) {
		camera_x += sin_yaw * move_speed;
		camera_z += cos_yaw * move_speed;
	}
	if (key_s) {
		camera_x -= sin_yaw * move_speed;
		camera_z -= cos_yaw * move_speed;
	}
	if (key_a) {
		camera_x -= cos_yaw * move_speed;
		camera_z += sin_yaw * move_speed;
	}
	if (key_d) {
		camera_x += cos_yaw * move_speed;
		camera_z -= sin_yaw * move_speed;
	}
	if (key_q) {
		camera_y -= move_speed;
	}
	if (key_e) {
		camera_y += move_speed;
	}
	
	constants_type *c = constants_type_buffer_lock(&constants, 0, 1);
	c->mvp = kore_matrix4x4_identity();
	constants_type_buffer_unlock(&constants);

	rc_type *rc = rc_type_buffer_lock(&rc_buffer, 0, 1);
	rc->camera.x = camera_x;
	rc->camera.y = camera_y;
	rc->camera.z = camera_z;
	rc->camera.w = camera_yaw;
	rc->light.x = 0.0f;
	rc->light.y = 200.0f;
	rc->light.z = 0.0f;
	rc->light.w = camera_pitch;
	rc->sky_top.x = 0.24f;
	rc->sky_top.y = 0.44f;
	rc->sky_top.z = 0.72f;
	rc->sky_top.w = 0.0f;
	rc->sky_bottom.x = 0.75f;
	rc->sky_bottom.y = 0.86f;
	rc->sky_bottom.z = 0.93f;
	rc->sky_bottom.w = 0.0f;
	rc_type_buffer_unlock(&rc_buffer);

	kore_gpu_texture *framebuffer = kore_gpu_device_get_framebuffer(&device);

	kong_set_compute_shader_comp(&list);
	kong_set_descriptor_set_compute(&list, &compute);
	kore_gpu_command_list_compute(&list, (width + 15) / 16, (height + 15) / 16, 1);

	kore_gpu_render_pass_parameters parameters = {0};
	parameters.color_attachments_count = 1;
	parameters.color_attachments[0].load_op = KORE_GPU_LOAD_OP_CLEAR;
	kore_gpu_color clear_color = {0.0f, 0.0f, 0.25f, 1.0f};
	parameters.color_attachments[0].clear_value = clear_color;
	parameters.color_attachments[0].texture.texture = framebuffer;
	parameters.color_attachments[0].texture.array_layer_count = 1;
	parameters.color_attachments[0].texture.mip_level_count = 1;
	parameters.color_attachments[0].texture.format = kore_gpu_device_framebuffer_format(&device);
	parameters.color_attachments[0].texture.dimension = KORE_GPU_TEXTURE_VIEW_DIMENSION_2D;
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

	kore_keyboard_set_key_down_callback(key_down, NULL);
	kore_keyboard_set_key_up_callback(key_up, NULL);
	kore_mouse_set_move_callback(mouse_move, NULL);

	kore_gpu_device_wishlist wishlist = {0};
	kore_gpu_device_create(&device, &wishlist);
	kong_init(&device);
	kore_gpu_device_create_command_list(&device, KORE_GPU_COMMAND_LIST_TYPE_GRAPHICS, &list);

	kore_gpu_texture_parameters texture_params = {
	    .width = width,
	    .height = height,
	    .depth_or_array_layers = 1,
	    .mip_level_count = 1,
	    .sample_count = 1,
	    .dimension = KORE_GPU_TEXTURE_DIMENSION_2D,
	    .format = KORE_GPU_TEXTURE_FORMAT_RGBA32_FLOAT,
	    .usage = KORE_GPU_TEXTURE_USAGE_SAMPLED | KORE_GPU_TEXTURE_USAGE_STORAGE,
	};
	kore_gpu_device_create_texture(&device, &texture_params, &render_target_texture);

	kore_gpu_sampler_parameters sampler_params = {
	    .address_mode_u = KORE_GPU_ADDRESS_MODE_REPEAT,
	    .address_mode_v = KORE_GPU_ADDRESS_MODE_REPEAT,
	    .address_mode_w = KORE_GPU_ADDRESS_MODE_REPEAT,
	    .mag_filter = KORE_GPU_FILTER_MODE_LINEAR,
	    .min_filter = KORE_GPU_FILTER_MODE_LINEAR,
	    .mipmap_filter = KORE_GPU_MIPMAP_FILTER_MODE_NEAREST,
	    .lod_min_clamp = 1,
	    .lod_max_clamp = 32,
	    .compare = KORE_GPU_COMPARE_FUNCTION_UNDEFINED,
	    .max_anisotropy = 1,
	};
	kore_gpu_device_create_sampler(&device, &sampler_params, &sampler);

	kong_create_buffer_vertex_in(&device, 4, &vertices);
	{
		vertex_in *v = kong_vertex_in_buffer_lock(&vertices);
		v[0].pos.x = -1.0f; v[0].pos.y = -1.0f; v[0].pos.z = 0.5f;
		v[0].tex.x = 0.0f; v[0].tex.y = 1.0f;
		v[1].pos.x = 1.0f; v[1].pos.y = -1.0f; v[1].pos.z = 0.5f;
		v[1].tex.x = 1.0f; v[1].tex.y = 1.0f;
		v[2].pos.x = -1.0f; v[2].pos.y = 1.0f; v[2].pos.z = 0.5f;
		v[2].tex.x = 0.0f; v[2].tex.y = 0.0f;
		v[3].pos.x = 1.0f; v[3].pos.y = 1.0f; v[3].pos.z = 0.5f;
		v[3].tex.x = 1.0f; v[3].tex.y = 0.0f;
		kong_vertex_in_buffer_unlock(&vertices);
	}

	{
		kore_gpu_buffer_parameters params = {
		    .size = 6 * sizeof(uint16_t),
		    .usage_flags = KORE_GPU_BUFFER_USAGE_INDEX | KORE_GPU_BUFFER_USAGE_CPU_WRITE,
		};
		kore_gpu_device_create_buffer(&device, &params, &indices);
		uint16_t *id = (uint16_t *)kore_gpu_buffer_lock_all(&indices);
		id[0] = 0; id[1] = 1; id[2] = 2;
		id[3] = 2; id[4] = 1; id[5] = 3;
		kore_gpu_buffer_unlock(&indices);
	}

	constants_type_buffer_create(&device, &constants, 1);
	rc_type_buffer_create(&device, &rc_buffer, 1);
	vertex_buffer_type_buffer_create(&device, &vertex_buffer_storage, 1);
	index_buffer_type_buffer_create(&device, &index_buffer_storage, 1);

	{
		everything_parameters params = {
		    .constants = &constants,
		    .render_target = {
		        .texture = &render_target_texture,
		        .base_mip_level = 0,
		        .mip_level_count = 1,
		        .array_layer_count = 1,
		    },
		    .render_target_sampler = &sampler,
		};
		kong_create_everything_set(&device, &params, &everything);
	}

	{
		compute_parameters params = {
		    .rc = &rc_buffer,
		    .vertex_buffer = &vertex_buffer_storage,
		    .index_buffer = &index_buffer_storage,
		    .render_target = {
		        .texture = &render_target_texture,
		        .base_mip_level = 0,
		        .mip_level_count = 1,
		        .array_layer_count = 1,
		    },
		};
		kong_create_compute_set(&device, &params, &compute);
	}

	kore_start();

	kong_destroy_compute_set(&compute);
	index_buffer_type_buffer_destroy(&index_buffer_storage);
	vertex_buffer_type_buffer_destroy(&vertex_buffer_storage);
	rc_type_buffer_destroy(&rc_buffer);
	constants_type_buffer_destroy(&constants);
	kong_destroy_everything_set(&everything);
	kore_gpu_buffer_destroy(&indices);
	kong_destroy_buffer_vertex_in(&vertices);
	kore_gpu_command_list_destroy(&list);
	kore_gpu_sampler_destroy(&sampler);
	kore_gpu_texture_destroy(&render_target_texture);
	kore_gpu_device_destroy(&device);

	return 0;
}
