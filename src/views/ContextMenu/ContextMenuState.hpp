//
//  ContextMenuState.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 11/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_ContextMenuState_hpp
#define ddui_ContextMenuState_hpp

#include <vector>
#include <string>

namespace ContextMenu {

struct Item {
    std::string label;
    bool checked;
};

struct ContextMenuState {
    ContextMenuState();

    bool open;
    void* identifier;
    int action_pressing;
    int action;
    int x, y;
    std::vector<Item> items;
};

}

#endif
