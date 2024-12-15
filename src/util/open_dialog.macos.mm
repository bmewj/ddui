//
//  open_dialog.macos.mm
//  ddui
//
//  Created by Bartholomew Joyce on 30/10/2024.
//  Copyright © 2024 Bartholomew Joyce All rights reserved.
//

#include "open_dialog.hpp"
#include "../glfw.hpp"
#include <AppKit/AppKit.h>
#include <vector>
#include <string>
#include <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

static const void* dialog_identifier = nullptr;
static NSOpenPanel* panel = nullptr;
static bool dialog_open = false;
static bool dialog_done = false;
static std::vector<std::string> file_paths;

void ddui::open_dialog::show(const void* identifier, const Properties& properties) {

    // Created panel?
    if (panel == nullptr) {
        panel = [NSOpenPanel openPanel];
        [panel retain];
    }

    // Already open?
    if (dialog_open) {
        return;
    }

    // Reset
    dialog_identifier = identifier;
    dialog_done = false;
    file_paths.clear();

    // Configure the panel
    panel.title = [NSString stringWithUTF8String:properties.title];
    panel.message = [NSString stringWithUTF8String:properties.message];
    panel.canChooseFiles = properties.can_choose_files;
    panel.canChooseDirectories = properties.can_choose_directories;
    panel.allowsMultipleSelection = properties.can_choose_multiple;
    if (properties.allowed_file_types != nullptr) {
        NSMutableArray<UTType*>* fileTypesArray = [NSMutableArray array];
        for (const char** current = properties.allowed_file_types; *current != nullptr; ++current) {
            NSString* fileType = [NSString stringWithUTF8String:*current];
            UTType* contentType = [UTType typeWithFilenameExtension:fileType];
            if (contentType != nil) {
                [fileTypesArray addObject:contentType];
            }
        }
        panel.allowedContentTypes = [fileTypesArray copy];
    }

    // Open the panel
    [panel beginWithCompletionHandler:^(NSModalResponse result) {
        dialog_open = false;
        dialog_done = true;
        if (result == NSModalResponseOK) {
            for (NSURL* url in panel.URLs) {
                file_paths.push_back(std::string([[url path] UTF8String]));
            }
        }

        ddui::focus_main_window();
        ddui::repaint("");
    }];
}

bool ddui::open_dialog::process_files(const void* identifier, size_t* num_files_out, const char*** file_paths_out) {

    if (!dialog_done || dialog_identifier != identifier) {
        return false;
    }

    static std::vector<const char*> file_paths_c;
    file_paths_c.clear();
    file_paths_c.reserve(file_paths.size());
    for (auto& file_path : file_paths) {
        file_paths_c.push_back(file_path.c_str());
    }

    *num_files_out = file_paths.size();
    if (*num_files_out == 0) {
        *file_paths_out = nullptr;
    } else {
        *file_paths_out = &file_paths_c[0];
    }
    dialog_done = false;

    return true;
}
