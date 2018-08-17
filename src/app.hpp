//
//  app.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 11/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_app_hpp
#define ddui_app_hpp

#include "Context.hpp"

namespace app {

bool init(const char* title_bar);
void load_font_face(const char* name, const char* file_name);
void run(void (*update_function)(Context));
void terminate();
void post_empty_event();
void set_immediate(std::function<void()> callback);
const char* get_clipboard_string();
void set_clipboard_string(const char* string);

}

#endif
