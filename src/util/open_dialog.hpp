//
//  open_dialog.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 30/10/2024.
//  Copyright © 2024 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_open_dialog_hpp
#define ddui_open_dialog_hpp

#include <ddui/core>

namespace ddui {
namespace open_dialog {

struct Properties {
    const char* title = "Choose a file";
    const char* message = "Select a file to open.";
    bool can_choose_files = true;
    bool can_choose_directories = false;
    bool can_choose_multiple = false;
    const char** allowed_file_types = nullptr;
};

void show(const void* identifier, const Properties& properties);
bool process_files(const void* identifier, size_t* num_files_out, const char*** file_paths_out);

}
}

#endif
