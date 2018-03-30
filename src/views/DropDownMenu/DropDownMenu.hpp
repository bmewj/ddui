//
//  DropDownMenu.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 23/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_DropDownMenu_hpp
#define ddui_DropDownMenu_hpp

#include "DropDownMenuState.hpp"
#include <ddui/Context>
#include <functional>

namespace DropDownMenu {

void update(State* state, Context ctx, std::function<void(Context)> inner_update);

int process_action(Context ctx, void* identifier);
void show(Context ctx, void* identifier, int x, int y, std::vector<Item> items);

}

#endif
