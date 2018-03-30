//
//  DropDownMenuState.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 23/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_DropDownMenuState_hpp
#define ddui_DropDownMenuState_hpp

#include <vector>
#include <string>
#include "../ScrollArea/ScrollAreaState.hpp"

namespace DropDownMenu {

struct Item {
    std::string label;
    bool checked = false;
    bool is_separator = false;
};

struct State {
    bool open = false;
    void* identifier = NULL;
    int action_pressing;
    int action;
    int x, y;
    std::vector<Item> items;

    ScrollArea::ScrollAreaState scroll_area_state;
};

}

#endif
