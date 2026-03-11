#ifndef KORE_UI_HEADER
#define KORE_UI_HEADER

#include <kore3/global.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KORE_UI_DIALOG_OK        0
#define KORE_UI_DIALOG_OK_CANCEL 1
#define KORE_UI_DIALOG_YES_NO    2

#define KORE_UI_DIALOG_INFO     0
#define KORE_UI_DIALOG_ERROR    1
#define KORE_UI_DIALOG_WARNING  2
#define KORE_UI_DIALOG_QUESTION 3

#define KORE_UI_FILE_CHOOSER_OK     0
#define KORE_UI_FILE_CHOOSER_CANCEL 1

typedef struct kore_ui_file_chooser_options {
    const char *title;
    const char *initial_directory;
    const char *file_name;
    const char **filters;
    int filter_count;
    bool for_save;
    int window_id;
} kore_ui_file_chooser_options;

KORE_FUNC int kore_ui_dialog(const char *title, const char *message, int dialog_type, int icon);

KORE_FUNC int kore_ui_file_chooser(kore_ui_file_chooser_options *options, char *buffer, int buffer_size);

KORE_FUNC bool kore_ui_clipboard_set_text(const char *text);

KORE_FUNC char *kore_ui_clipboard_get_text(void);

#ifdef __cplusplus
}
#endif

#endif
