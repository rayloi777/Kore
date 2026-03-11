#import <kore3/ui.h>

#import <AppKit/AppKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

int kore_ui_dialog(const char *title, const char *message, int dialog_type, int icon) {
    NSAlert *alert = [[NSAlert alloc] init];
    alert.messageText = [NSString stringWithUTF8String:title];
    alert.informativeText = [NSString stringWithUTF8String:message];

    switch (icon) {
        case KORE_UI_DIALOG_ERROR:
            alert.alertStyle = NSAlertStyleCritical;
            break;
        case KORE_UI_DIALOG_WARNING:
            alert.alertStyle = NSAlertStyleWarning;
            break;
        case KORE_UI_DIALOG_QUESTION:
            alert.alertStyle = NSAlertStyleWarning;
            break;
        default:
            alert.alertStyle = NSAlertStyleInformational;
            break;
    }

    switch (dialog_type) {
        case KORE_UI_DIALOG_OK_CANCEL:
            [alert addButtonWithTitle:@"OK"];
            [alert addButtonWithTitle:@"Cancel"];
            break;
        case KORE_UI_DIALOG_YES_NO:
            [alert addButtonWithTitle:@"Yes"];
            [alert addButtonWithTitle:@"No"];
            break;
        default:
            [alert addButtonWithTitle:@"OK"];
            break;
    }

    NSModalResponse response = [alert runModal];
    if (response == NSAlertFirstButtonReturn) {
        return 1;
    }
    return 0;
}

int kore_ui_file_chooser(kore_ui_file_chooser_options *options, char *buffer, int buffer_size) {
    NSOpenPanel *panel;

    if (options->for_save) {
        panel = (NSOpenPanel *)[NSSavePanel savePanel];
    } else {
        panel = [NSOpenPanel openPanel];
        [(NSOpenPanel *)panel setAllowsMultipleSelection:NO];
        [(NSOpenPanel *)panel setCanChooseDirectories:NO];
        [(NSOpenPanel *)panel setCanChooseFiles:YES];
    }

    if (options->title != NULL) {
        panel.title = [NSString stringWithUTF8String:options->title];
    }

    if (options->initial_directory != NULL) {
        NSString *dir = [NSString stringWithUTF8String:options->initial_directory];
        panel.directoryURL = [NSURL fileURLWithPath:dir];
    }

    if (options->file_name != NULL && options->for_save) {
        [(NSSavePanel *)panel setNameFieldStringValue:[NSString stringWithUTF8String:options->file_name]];
    }

    if (options->filters != NULL && options->filter_count > 0) {
        NSMutableArray<UTType *> *types = [NSMutableArray array];
        for (int i = 0; i < options->filter_count; i++) {
            const char *filter = options->filters[i];
            NSString *nsfilter = [NSString stringWithUTF8String:filter];
            UTType *type = [UTType typeWithFilenameExtension:nsfilter];
            if (type != nil) {
                [types addObject:type];
            }
        }
        if (types.count > 0) {
            panel.allowedContentTypes = types;
        }
    }

    NSModalResponse result = [panel runModal];

    if (result == NSModalResponseOK) {
        NSURL *url = panel.URL;
        if (url != NULL && buffer != NULL && buffer_size > 0) {
            NSString *path = url.path;
            const char *path_cstr = [path UTF8String];
            strncpy(buffer, path_cstr, buffer_size - 1);
            buffer[buffer_size - 1] = '\0';
            return KORE_UI_FILE_CHOOSER_OK;
        }
    }

    return KORE_UI_FILE_CHOOSER_CANCEL;
}

bool kore_ui_clipboard_set_text(const char *text) {
    NSPasteboard *board = [NSPasteboard generalPasteboard];
    [board clearContents];
    BOOL result = [board setString:[NSString stringWithUTF8String:text] forType:NSPasteboardTypeString];
    return result != NO;
}

char *kore_ui_clipboard_get_text(void) {
    NSPasteboard *board = [NSPasteboard generalPasteboard];
    NSString *text = [board stringForType:NSPasteboardTypeString];
    if (text == nil) {
        return NULL;
    }
    const char *cstr = [text UTF8String];
    if (cstr == NULL) {
        return NULL;
    }
    char *result = (char *)malloc(strlen(cstr) + 1);
    if (result != NULL) {
        strcpy(result, cstr);
    }
    return result;
}
