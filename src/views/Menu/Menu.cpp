//
//  Menu.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 25/07/2019.
//  Copyright © 2019 Bartholomew Joyce All rights reserved.
//

#include "Menu.hpp"
#include <ddui/core>

Menu::Menu(State& state) : state(state) {
    
}

Menu& Menu::process_user_input(Action* action) {

    // This function can be called without providing an action,
    // let's handle this gracefully
    Action stack_action;
    if (action == NULL) {
        action = &stack_action;
    }

    // There's no point processing a menu that has no items
    if (state.opened_menu_stack.empty()) {
        action->type = NO_ACTION;
        return *this;
    }

    // Let's lay out the menus inside the view
    auto num_opened_menus = state.opened_menu_stack.size();
    {
        Anchor a;
        a.direction = Anchor::LEFT_TO_RIGHT;
        a.x = state.root_x;
        a.y = state.root_y;

        Anchor b;
        b.direction = Anchor::RIGHT_TO_LEFT;
        b.x = state.root_x + state.root_width;
        b.y = state.root_y;

        auto& root_menu = state.opened_menu_stack.front();
        lay_out_menu(a, b, root_menu);
    }
    for (int i = 1; i < num_opened_menus; ++i) {
        const auto& parent_menu = state.opened_menu_stack[i - 1];
        
        Anchor a, b;
        parent_menu.view_state->get_item_anchors(
            state.menus[parent_menu.menu_index],
            parent_menu.bounding_rect,
            parent_menu.selected_item_index,
            &a,
            &b
        );

        // If the parent menu was opened in a RIGHT_TO_LEFT
        // fashion, we want to try and do the same for the child
        // menu as well by swapping our a and b anchors.
        if (parent_menu.anchor.direction != a.direction) {
            std::swap(a, b);
        }

        auto& menu = state.opened_menu_stack[i];
        lay_out_menu(a, b, menu);
    }

    // Where is the mouse positioned?
    float mx, my;
    ddui::mouse_position(&mx, &my);

    // Let's try and find the item that the mouse is hovering on
    int active_menu_index = -1;
    int active_item_index = -1;
    {
        for (int i = num_opened_menus - 1; i >= 0; --i) {
            auto& opened_menu = state.opened_menu_stack[i];

            const auto& bounds = opened_menu.bounding_rect;
            if (!(mx >= bounds.x && mx < bounds.x + bounds.width &&
                  my >= bounds.y && my < bounds.y + bounds.height)) {
                continue; // Not hovering over menu
            }

            const auto& menu = state.menus[opened_menu.menu_index];

            active_menu_index = i;
            active_item_index = opened_menu.view_state->process_user_input(menu, opened_menu.bounding_rect);
            break;
        }
    }

    // If the user clicks and no menu is hovering, the menus should be dismissed
    if (ddui::mouse_hit(0, 0, ddui::view.width, ddui::view.height)) {
        if (active_menu_index == -1) {
            action->type = MENU_DISMISS;
            return *this;
        } else {
            ddui::mouse_hit_accept();
            state.mouse_is_pressed = true;
        }
    }

    // If the user releases a press, we will create an action
    bool mouse_is_pressed = ddui::mouse_state.pressed || ddui::mouse_state.pressed_secondary;
    bool naive_did_release = (state.mouse_is_pressed && !mouse_is_pressed);
    bool did_release = false;
    if (!state.mouse_is_first_press) {
        did_release = naive_did_release;
    } else if (naive_did_release) {
        float mx, my, dx, dy;
        ddui::mouse_movement(&mx, &my, &dx, &dy);
        did_release = (dx != 0 || dy != 0);
        state.mouse_is_first_press = false;
    }
    if (did_release) {
        state.mouse_is_pressed = false;

        if (active_menu_index == -1 || active_item_index == -1) {
            action->type = MENU_DISMISS;
        } else {
            const auto  menu_index = state.opened_menu_stack[active_menu_index].menu_index;
            const auto& menu = state.menus[menu_index];
            const auto& item = menu.items[active_item_index];

            if (!item.separator && item.action_index != 0) {
                action->type = ITEM_CLICK;
                action->menu_index = menu_index;
                action->item_index = active_item_index;
                if (item.action_index < 0) {
                    state.action_callbacks[- item.action_index - 1]();
                    action->action_id = -1;
                } else {
                    action->action_id = state.action_ids[item.action_index - 1];
                }
                return *this;
            }
        }
    }
    state.mouse_is_pressed = mouse_is_pressed;

    // From here on forward, active_menu_index will not be -1 for no hovers, we will pick
    // a sub menu to be active
    if (active_menu_index == -1) {
        active_menu_index = num_opened_menus - 1;
    }

    // We definitely want to close all menus that are grand-children of the selected menu
    for (int i = num_opened_menus - 1; i > active_menu_index + 1; --i) {
        state.opened_menu_stack.pop_back();
        --num_opened_menus;
    }

    // If we still have a child of the selected menu open, it should only stay open if
    // the mouse is hovering on the specific menu item in the parent menu that triggers
    // the submenu.
    if (active_menu_index == num_opened_menus - 2) {
        if (state.opened_menu_stack[active_menu_index].selected_item_index != active_item_index) {
            state.opened_menu_stack.pop_back();
            --num_opened_menus;
        } else {
            state.opened_menu_stack.back().selected_item_index = -1;
        }
    }
    state.opened_menu_stack[active_menu_index].selected_item_index = active_item_index;

    // Now if there is a menu item selected that has a sub-menu, we want to open this menu
    const auto& menu = state.menus[state.opened_menu_stack[active_menu_index].menu_index];
    if (active_menu_index == num_opened_menus - 1 &&
        active_item_index != -1 &&
        menu.items[active_item_index].sub_menu_index != -1) {

        state.opened_menu_stack.push_back(OpenedMenuState());
        ++num_opened_menus;

        auto& parent_menu = state.opened_menu_stack[num_opened_menus - 2];
        auto& sub_menu = state.opened_menu_stack[num_opened_menus - 1];

        sub_menu.menu_index = menu.items[active_item_index].sub_menu_index;
        sub_menu.selected_item_index = -1;
        sub_menu.view_state = std::unique_ptr<ISubMenuView>(menu.construct_view_state());

        Anchor a, b;
        parent_menu.view_state->get_item_anchors(
            state.menus[parent_menu.menu_index],
            parent_menu.bounding_rect,
            active_item_index,
            &a,
            &b
        );

        // If the parent menu was opened in a RIGHT_TO_LEFT
        // fashion, we want to try and do the same for the child
        // menu as well by swapping our a and b anchors.
        if (parent_menu.anchor.direction != a.direction) {
            std::swap(a, b);
        }

        lay_out_menu(a, b, sub_menu);
    }

    return *this;
}

Menu& Menu::render() {

    using namespace ddui;

    for (const auto& opened_menu : state.opened_menu_stack) {
        const auto& menu = state.menus[opened_menu.menu_index];
        opened_menu.view_state->render(menu, opened_menu.bounding_rect, opened_menu.selected_item_index);
    }

    return *this;
}

void Menu::lay_out_menu(const Anchor& a, const Anchor& b, OpenedMenuState& opened_menu) {

    auto& menu = state.menus[opened_menu.menu_index];
    
    BoundingRect  bounds_in;
    BoundingRect& bounds_out = opened_menu.bounding_rect;
    
    // Step 1. Calculate the available width for the menu
    auto a_space = (a.direction == Anchor::LEFT_TO_RIGHT) ? ddui::view.width - a.x : a.x;
    auto b_space = (b.direction == Anchor::LEFT_TO_RIGHT) ? ddui::view.width - b.x : b.x;
    auto available_width = (a_space > b_space) ? a_space : b_space;

    // Step 2. Get the view renderer to tell us the bounds of the menu
    opened_menu.view_state->get_bounding_rect(menu, available_width, ddui::view.height, &bounds_in);
    bounds_out.width  = bounds_in.width;
    bounds_out.height = bounds_in.height;

    // Step 3. Try and pick an anchor where the whole width will fit
    choose_most_suitable_anchor(a, b, bounds_in.width, &opened_menu.anchor);

    // Step 4. Calculate the top-left x coordinate
    bounds_out.x = opened_menu.anchor.x;
    if (opened_menu.anchor.direction == Anchor::RIGHT_TO_LEFT) {
        bounds_out.x -= bounds_out.width;
    }
    bounds_out.x -= bounds_in.x; // align the first item to the anchor

    // Step 5. Calculate the top-left y coordinate
    float space = ddui::view.height - opened_menu.anchor.y + bounds_in.y;
    if (space >= bounds_out.height) {
        bounds_out.y = opened_menu.anchor.y - bounds_in.y;
    } else {
        bounds_out.y = ddui::view.height - bounds_out.height;
    }
    if (bounds_out.y < 0) {
        bounds_out.y = 0;
    }

}

void Menu::choose_most_suitable_anchor(const Anchor& a, const Anchor& b, float width, Anchor* out) {
    // Pick Anchor a if possible
    float a_space = (a.direction == Anchor::LEFT_TO_RIGHT) ? ddui::view.width - a.x : a.x;
    if (a_space >= width) {
        *out = a;
        return;
    }

    // Pick Anchor b if possible
    float b_space = (b.direction == Anchor::LEFT_TO_RIGHT) ? ddui::view.width - b.x : b.x;
    if (b_space >= width) {
        *out = b;
        return;
    }

    // Pick the biggest space if short on space
    if (a_space > b_space) {
        *out = a;
    } else {
        *out = b;
    }
}
