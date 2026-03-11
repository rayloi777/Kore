#include <kore3/gpu/device.h>
#include <kore3/io/filereader.h>
#include <kore3/system.h>
#include <kore3/ui.h>

#include <kong.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static kore_gpu_device       device;
static kore_gpu_command_list list;

static const int width  = 800;
static const int height = 600;

static int dialog_shown = 0;

static void update(void *data) {
	if (dialog_shown == 0) {
		dialog_shown = 1;
		printf("[UI Test] Showing dialog...\n");
		int result = kore_ui_dialog("UI Test", "Dialogs are working!", KORE_UI_DIALOG_OK_CANCEL, KORE_UI_DIALOG_INFO);
		printf("[UI Test] Dialog result: %d\n", result);
		
		printf("[UI Test] Testing clipboard set...\n");
		kore_ui_clipboard_set_text("Hello from Kore UI!");
		printf("[UI Test] Clipboard set done\n");
		
		printf("[UI Test] Testing clipboard get...\n");
		char *text = kore_ui_clipboard_get_text();
		if (text != NULL) {
			printf("[UI Test] Clipboard content: %s\n", text);
			free(text);
		}
		
		printf("[UI Test] Testing file chooser...\n");
		kore_ui_file_chooser_options opts = {
			.title = "Select a File",
			.for_save = false,
		};
		char path[1024];
		int file_result = kore_ui_file_chooser(&opts, path, sizeof(path));
		if (file_result == KORE_UI_FILE_CHOOSER_OK) {
			printf("[UI Test] Selected: %s\n", path);
		} else {
			printf("[UI Test] File chooser cancelled\n");
		}
	}

	kore_gpu_texture *framebuffer = kore_gpu_device_get_framebuffer(&device);

	kore_gpu_color clear_color = {
	    .r = 0.1f,
	    .g = 0.4f,
	    .b = 0.8f,
	    .a = 1.0f,
	};

	kore_gpu_render_pass_parameters parameters = {
	    .color_attachments_count = 1,
	    .color_attachments =
	        {
	            {
	                .load_op     = KORE_GPU_LOAD_OP_CLEAR,
	                .clear_value = clear_color,
	                .texture =
	                    {
	                        .texture           = framebuffer,
	                        .array_layer_count = 1,
	                        .mip_level_count   = 1,
	                        .format            = KORE_GPU_TEXTURE_FORMAT_BGRA8_UNORM,
	                        .dimension         = KORE_GPU_TEXTURE_VIEW_DIMENSION_2D,
	                    },
	            },
	        },
	};
	kore_gpu_command_list_begin_render_pass(&list, &parameters);

	kore_gpu_command_list_end_render_pass(&list);

	kore_gpu_command_list_present(&list);

	kore_gpu_device_execute_command_list(&device, &list);
}

int kickstart(int argc, char **argv) {
	kore_init("UI Test", width, height, NULL, NULL);

	printf("[UI Test] Starting... Dialogs will appear automatically.\n");

	kore_set_update_callback(update, NULL);

	kore_gpu_device_wishlist wishlist = {0};
	kore_gpu_device_create(&device, &wishlist);

	kong_init(&device);

	kore_gpu_device_create_command_list(&device, KORE_GPU_COMMAND_LIST_TYPE_GRAPHICS, &list);

	kore_start();

	return 0;
}
