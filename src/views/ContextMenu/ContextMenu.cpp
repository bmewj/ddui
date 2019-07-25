//
//  ContextMenu.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 11/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "ContextMenu.hpp"

namespace ContextMenu {

using namespace ddui;

struct ContextMenuState {
    ContextMenuState();

    bool open;
    void* identifier;
    int action;

    Menu::State menu_state;
};

ContextMenuState::ContextMenuState() {
    open = false;
    identifier = NULL;
    action = -1;
}

static ContextMenuState state;

void update(std::function<void()> inner_update) {
    if (!state.open) {
        inner_update();
        return;
    }

    Menu menu(state.menu_state);

    // Process menu user input
    Menu::Action action;
    menu.process_user_input(&action);

    switch (action.type) {
        case Menu::NO_ACTION:
            break;
        case Menu::MENU_DISMISS:
            state.open = false;
            break;
        case Menu::ITEM_CLICK:
            state.open = false;
            state.action = action.action_id;
            break;
    }

    // Do inner update
    inner_update();

    // Render the menu
    menu.render();
}

int process_action(void* identifier) {
    if (!state.open && state.identifier == identifier) {
        state.identifier = NULL;
        return state.action;
    }

    return -1;
}

void show(void* identifier, float x, float y, MenuBuilder::Menu& menu) {
    state.open = true;
    state.identifier = identifier;
    state.action = -1;

    float gx, gy;
    to_global_position(&gx, &gy, x, y);
    state.menu_state = menu.builder.create(menu, gx, gy);
}

bool is_showing(void* identifier) {
    return (state.open && state.identifier == identifier);
}

}
