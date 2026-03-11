#include <kore3/ui.h>

#include <kore3/log.h>

#include <string.h>

#if !defined(KORE_WINDOWS) && !defined(KORE_LINUX) && !defined(KORE_MACOS)

int kore_ui_dialog(const char *title, const char *message, int dialog_type, int icon) {
	kore_log(KORE_LOG_LEVEL_WARNING, "kore_ui_dialog is not implemented for this system.");
	return 0;
}

int kore_ui_file_chooser(kore_ui_file_chooser_options *options, char *buffer, int buffer_size) {
	kore_log(KORE_LOG_LEVEL_WARNING, "kore_ui_file_chooser is not implemented for this system.");
	return KORE_UI_FILE_CHOOSER_CANCEL;
}

bool kore_ui_clipboard_set_text(const char *text) {
	kore_log(KORE_LOG_LEVEL_WARNING, "kore_ui_clipboard_set_text is not implemented for this system.");
	return false;
}

char *kore_ui_clipboard_get_text(void) {
	kore_log(KORE_LOG_LEVEL_WARNING, "kore_ui_clipboard_get_text is not implemented for this system.");
	return NULL;
}

#endif
