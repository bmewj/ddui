//
//  ContextMenu.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 11/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_ContextMenu_hpp
#define ddui_ContextMenu_hpp

#include <ddui/Context>
#include <string>
#include <vector>

namespace ContextMenu {

struct Item {
    std::string label;
    bool enabled = true;
    bool checked = false;
    bool is_separator = false;
};

int process_action(Context ctx, void* identifier);
void show(Context ctx, void* identifier, std::vector<Item> items);

}

#endif
