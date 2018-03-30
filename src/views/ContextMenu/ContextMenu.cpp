//
//  ContextMenu.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 11/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "ContextMenu.hpp"
#include <ddui/views/DropDownMenu>

namespace ContextMenu {

int process_action(Context ctx, void* identifier) {
    return DropDownMenu::process_action(ctx, identifier);
}

void show(Context ctx, void* identifier, std::vector<Item> items) {
    std::vector<DropDownMenu::Item> items2;
    items2.reserve(items.size());
    for (auto& item : items) {
        DropDownMenu::Item item2;
        item2.label = std::move(item.label);
        item2.checked = item.checked;
        items2.push_back(std::move(item2));
    }
    DropDownMenu::show(ctx, identifier, ctx.mouse->x, ctx.mouse->y, std::move(items2));
}

}
