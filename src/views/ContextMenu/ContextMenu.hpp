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
#include <functional>
#include <string>
#include <vector>

namespace ContextMenu {

struct Item {
    std::string label;
    bool checked;
};

void update(Context ctx, std::function<void(Context)> inner_update);

int process_action(Context ctx, void* identifier);
void show(Context ctx, void* identifier, int x, int y, std::vector<Item> items);

}

#endif
