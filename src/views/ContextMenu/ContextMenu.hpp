//
//  ContextMenu.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 11/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_ContextMenu_hpp
#define ddui_ContextMenu_hpp

#include <ddui/core>
#include <functional>
#include <string>
#include <vector>
#include <ddui/views/Menu>

namespace ContextMenu {

struct Item {
    std::string label;
    bool checked;
};

void update(std::function<void()> inner_update);

int process_action(void* identifier);
void show(void* identifier, float x, float y, MenuBuilder::Menu& menu);
bool is_showing(void* identifier);

}

#endif
