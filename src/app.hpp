//
//  app.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 30/08/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_app_hpp
#define ddui_app_hpp

#include <functional>

namespace ddui {

bool app_init(int window_width, int window_height, const char* title, std::function<void()> update_proc);
void app_run();

}

#endif
