//
//  DropDownMenu.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 11/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_DropDownMenu_hpp
#define ddui_DropDownMenu_hpp

#include <ddui/core>
#include <functional>
#include <ddui/views/Menu>

namespace DropDownMenu {

void update(std::function<void()> inner_update);

int process_action(void* identifier);
void show(void* identifier, float x, float y, MenuBuilder::Menu& menu);
bool is_showing(void* identifier);

}

#endif
