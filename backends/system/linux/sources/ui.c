#include <kore3/ui.h>

#include <kore3/log.h>

#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

int kore_ui_dialog(const char *title, const char *message, int dialog_type, int icon) {
    GtkMessageType msg_type = GTK_MESSAGE_INFO;
    GtkButtonsType buttons = GTK_BUTTONS_OK;

    switch (icon) {
        case KORE_UI_DIALOG_ERROR:
            msg_type = GTK_MESSAGE_ERROR;
            break;
        case KORE_UI_DIALOG_WARNING:
            msg_type = GTK_MESSAGE_WARNING;
            break;
        case KORE_UI_DIALOG_QUESTION:
            msg_type = GTK_MESSAGE_QUESTION;
            break;
        default:
            msg_type = GTK_MESSAGE_INFO;
            break;
    }

    switch (dialog_type) {
        case KORE_UI_DIALOG_OK_CANCEL:
            buttons = GTK_BUTTONS_OK_CANCEL;
            break;
        case KORE_UI_DIALOG_YES_NO:
            buttons = GTK_BUTTONS_YES_NO;
            break;
        default:
            buttons = GTK_BUTTONS_OK;
            break;
    }

    GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, msg_type, buttons, "%s", message);
    gtk_window_set_title(GTK_WINDOW(dialog), title);

    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    if (response == GTK_RESPONSE_OK || response == GTK_RESPONSE_YES || response == GTK_RESPONSE_ACCEPT) {
        return 1;
    }
    return 0;
}

int kore_ui_file_chooser(kore_ui_file_chooser_options *options, char *buffer, int buffer_size) {
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    const char *dialog_title = "Choose File";

    if (options->for_save) {
        action = GTK_FILE_CHOOSER_ACTION_SAVE;
    }

    if (options->title != NULL) {
        dialog_title = options->title;
    }

    GtkWidget *dialog = gtk_file_chooser_dialog_new(dialog_title, NULL, action, "_Cancel", GTK_RESPONSE_CANCEL, NULL);

    if (options->for_save) {
        gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
    }

    if (options->initial_directory != NULL) {
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), options->initial_directory);
    }

    if (options->file_name != NULL && options->for_save) {
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), options->file_name);
    }

    if (options->filters != NULL && options->filter_count > 0) {
        for (int i = 0; i < options->filter_count; i++) {
            GtkFileFilter *filter = gtk_file_filter_new();
            gtk_file_filter_set_name(filter, options->filters[i]);
            gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
        }
    }

    gint response = gtk_dialog_run(GTK_DIALOG(dialog));

    if (response == GTK_RESPONSE_OK || response == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (filename != NULL && buffer != NULL && buffer_size > 0) {
            strncpy(buffer, filename, buffer_size - 1);
            buffer[buffer_size - 1] = '\0';
            g_free(filename);
            gtk_widget_destroy(dialog);
            return KORE_UI_FILE_CHOOSER_OK;
        }
        if (filename != NULL) {
            g_free(filename);
        }
    }

    gtk_widget_destroy(dialog);
    return KORE_UI_FILE_CHOOSER_CANCEL;
}

bool kore_ui_clipboard_set_text(const char *text) {
    GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_set_text(clipboard, text, -1);
    return true;
}

char *kore_ui_clipboard_get_text(void) {
    GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gchar *text = gtk_clipboard_wait_for_text(clipboard);
    if (text == NULL) {
        return NULL;
    }
    char *result = (char *)malloc(strlen(text) + 1);
    if (result != NULL) {
        strcpy(result, text);
    }
    g_free(text);
    return result;
}
