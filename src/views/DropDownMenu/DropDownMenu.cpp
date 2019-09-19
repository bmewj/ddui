//
//  DropDownMenu.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 11/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "DropDownMenu.hpp"

namespace DropDownMenu {

using namespace ddui;

struct DropDownMenuState {
    DropDownMenuState();

    bool open;
    void* identifier;
    int action;
    float gx, gy;

    Menu::State menu_state;
};

DropDownMenuState::DropDownMenuState() {
    open = false;
    identifier = NULL;
    action = -1;
}

static DropDownMenuState state;

void update(std::function<void()> inner_update) {
    if (!state.open) {
        inner_update();
        return;
    }

    // Update the positioning
    from_global_position(
        &state.menu_state.root_x,
        &state.menu_state.root_y,
        state.gx,
        state.gy
    );

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

    // Do the inner update and render the menu afterwards
    if (state.open) {
        menu.steal_user_input();
        inner_update();
        menu.render();
    } else {
        inner_update();
    }

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

    to_global_position(&state.gx, &state.gy, x, y);

    state.menu_state = menu.builder.create(menu, 0, 0);
}

bool is_showing(void* identifier) {
    return (state.open && state.identifier == identifier);
}

}
