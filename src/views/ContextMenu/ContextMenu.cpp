//
//  ContextMenu.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 19/09/2019.
//  Copyright © 2019 Bartholomew Joyce All rights reserved.
//

#include "ContextMenu.hpp"

struct ContextMenuState {
    bool is_open = false;
    float gx, gy;
    Menu::State menu_state;
    const ContextMenu::Handler* head_handler;
};

static ContextMenuState state;

ContextMenu::Handler::Handler(const std::function<void(MenuBuilder::Menu&)>& handler_fn)
 : handler_fn(handler_fn) {
    parent_handler = state.head_handler;
    state.head_handler = this;
    should_activate = !state.is_open && ddui::mouse_hit_secondary(0, 0, ddui::view.width, ddui::view.height);
}

ContextMenu::Handler::~Handler() {
    // When destructing a Handler, it should always be done in the reverse order that they were constructed.
    // In effect, we are creating a singly linked list allocated on the stack of handlers. Once a right-click
    // event occurs, we travel down this linked list to construct a single context menu containing all menu
    // items.
    if (state.head_handler != this) {
        printf("ERROR! ContextMenu::Handler destructed in the wrong order (should be perfect FIFO ordering)\n");
        exit(1);
    }

    // Can we open the menu?
    if (should_activate && !ddui::mouse_state.accepted) {
        ddui::mouse_hit_accept();
        ContextMenu::open();
    }

    // Change the head_handler pointer so that "this" gets removed from the linked list.
    state.head_handler = parent_handler;
}

void ContextMenu::update(const std::function<void()>& inner_update) {
    if (!state.is_open) {
        inner_update();
        if (state.is_open) {
            ddui::from_global_position(
                &state.menu_state.root_x,
                &state.menu_state.root_y,
                state.gx,
                state.gy
            );
            Menu(state.menu_state).render();
        }
        return;
    }

    // Update the positioning
    ddui::from_global_position(
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
        case Menu::ITEM_CLICK:
            state.is_open = false;
            break;
    }

    // Do the inner update and render the menu afterwards
    if (state.is_open) {
        menu.steal_user_input();
        inner_update();
        menu.render();
    } else {
        inner_update();
    }
}

void ContextMenu::open() {
    // Empty menu? No menu.
    if (state.head_handler == NULL) {
        state.is_open = false;
        return;
    }

    state.is_open = true;

    float mx, my;
    ddui::mouse_position(&mx, &my);
    ddui::to_global_position(&state.gx, &state.gy, mx, my);

    // Construct the menu
    MenuBuilder mb;
    auto menu = mb.menu();
    const Handler* ptr = state.head_handler;
    {
        ptr->handler_fn(menu);
        ptr = ptr->parent_handler;
    }
    while (ptr != NULL) {
        menu.separator();
        ptr->handler_fn(menu);
        ptr = ptr->parent_handler;
    }

    state.menu_state = menu.builder.create(menu, 0, 0);
}
