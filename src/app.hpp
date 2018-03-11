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
#include <functional>

namespace app {

bool init(const char* title_bar);
bool running();
void update(std::function<void(Context)> update_function);
void wait_events();
void terminate();
void post_empty_event();

}

#endif
