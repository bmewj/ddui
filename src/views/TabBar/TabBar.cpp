//
//  TabBar.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 27/06/2019.
//  Copyright © 2019 Bartholomew Joyce All rights reserved.
//

#include "TabBar.hpp"
#include <ddui/util/draw_text_in_box>
#include <ddui/views/ContextMenu>

using namespace ddui;

constexpr int MENU_ACTION_RENAME    = 0;
constexpr int MENU_ACTION_DUPLICATE = 1;
constexpr int MENU_ACTION_CLOSE     = 2;
constexpr double ANIMATION_DURATION = 0.2;

TabBar::TabBar(State& state) : state(state) {}

TabBar& TabBar::font(const char* font_face, float font_size) {
    this->font_face = font_face;
    this->font_size = font_size;
    return *this;
}

TabBar& TabBar::hover_background_color(ddui::Color color_bg_hover) {
    this->color_bg_hover = color_bg_hover;
    return *this;
}

TabBar& TabBar::text_color(ddui::Color color_text) {
    this->color_text = color_text;
    return *this;
}

TabBar& TabBar::text_color_hover(ddui::Color color_text_hover) {
    this->color_text_hover = color_text_hover;
    return *this;
}

TabBar& TabBar::text_color_active(ddui::Color color_text_active) {
    this->color_text_active = color_text_active;
    return *this;
}

TabBar& TabBar::new_tab_button_placement(int placement) {
    this->new_tab_placement = placement;
    return *this;
}

TabBar& TabBar::render(const std::vector<std::string>& tab_names, int active_tab) {
    new_tab_button_width = view.height;
    tab_width = view.width / tab_names.size();

    float tab_xs[tab_names.size()];
    this->tab_xs = tab_xs;

    // Create a subview excluding the space taken up by the new tab button
    if (new_tab_placement & align::LEFT) {
        sub_view(new_tab_button_width, 0, view.width - new_tab_button_width, view.height);
    } else {
        sub_view(0, 0, view.width - new_tab_button_width, view.height);
    }

    new_tab_button_x = (new_tab_placement & align::LEFT) ? - new_tab_button_width : view.width;

    diff_state(tab_names, active_tab);
    process_user_input();
    calculate_tab_positions();
    draw();

    restore();

    return *this;
}

void TabBar::process_action(Action* action) {
    *action = this->action;
}

void TabBar::diff_state(const std::vector<std::string>& tab_names, int active_tab) {
    if (state.tab_names.size() != tab_names.size()) {
        state.state = IDLE;
        if (!state.tab_names.empty()) {
            state.initial_tab_width = view.width / state.tab_names.size();
        } else {
            state.initial_tab_width = view.width / tab_names.size();
        }
        int removed_tab = find_removal(state.tab_names, tab_names);
        state.tab_names = tab_names;
        state.initial_tab_positions.clear();
        for (int i = 0; i < state.tab_names.size(); ++i) {
            state.initial_tab_positions.push_back(i);
        }
        if (removed_tab != -1) {
            for (int i = removed_tab; i < state.tab_names.size(); ++i) {
                state.initial_tab_positions[i]++;
            }
        }
    } else {
        for (int i = 0; i < state.tab_names.size(); ++i) {
            state.tab_names[i] = tab_names[i];
        }
    }
    state.active_tab = active_tab;
}

void TabBar::process_user_input() {
    action.type = NO_ACTION;
    float tab_width = view.width / state.tab_names.size();

    switch (state.state) {
        case IDLE: {
            // Clicking tabs
            float x = 0;
            for (int i = 0; i < state.tab_names.size(); ++i) {
                if (mouse_hit(x, 0, tab_width, view.height)) {
                    mouse_hit_accept();
                    state.state = TAB_PRESSED;
                    state.active_tab = i;
                    action.type = SWITCH_TO_TAB;
                    action.tab_index = i;
                    mouse_position(&state.initial_mx, &state.initial_my);
                    break;
                }
                if (mouse_hit_secondary(x, 0, tab_width, view.height)) {
                    mouse_hit_accept();
                    state.state = CONTEXT_MENU_SHOWING;
                    state.active_tab = i;
                    action.type = SWITCH_TO_TAB;
                    action.tab_index = i;
                    open_context_menu();
                    break;
                }
                x += tab_width;
            }
            if (mouse_hit(new_tab_button_x, 0, new_tab_button_width, view.height)) {
                mouse_hit_accept();
                action.type = NEW_TAB;
            }
            break;
        }
        case CONTEXT_MENU_SHOWING: {
            auto menu_action = ContextMenu::process_action(this);
            if (menu_action == MENU_ACTION_RENAME) {
               open_rename_input();
            } else if (menu_action == MENU_ACTION_DUPLICATE) {
                action.type = DUPLICATE_TAB;
                action.tab_index = state.active_tab;
                state.state = IDLE;
            } else if (menu_action == MENU_ACTION_CLOSE) {
                action.type = CLOSE_TAB;
                action.tab_index = state.active_tab;
                state.state = IDLE;
            } else if (!ContextMenu::is_showing(this)) {
                state.state = IDLE;
                state.active_tab = -1;
            }
            break;
        }
        case TAB_PRESSED: {
            if (!mouse_state.pressed) {
                state.state = IDLE;
                tab_drag_drop();
                break;
            }
            break;
        }
        case TAB_RENAMING: {
            bool should_close = false;

            if (did_blur(&state.text_box_state)) {
                should_close = true;
                ddui::repaint("TabBar::process_user_input");
            } else if (has_key_event(&state.text_box_state) &&
                       key_state.action == keyboard::ACTION_PRESS &&
                       (key_state.key == keyboard::KEY_ENTER ||
                        key_state.key == keyboard::KEY_KP_ENTER)) {
                consume_key_event();
                should_close = true;
            }

            if (should_close) {
                auto content = TextEdit::get_text_content(&state.text_box_model);
                auto text = std::string(content.get());
                if (!text.empty()) {
                    action.type = RENAME_TAB;
                    action.tab_index = state.active_tab;
                    action.new_name = text;
                }
                state.state = IDLE;
                state.active_tab = -1;
            }
            break;
        }
    }
}

void TabBar::calculate_tab_positions() {

    // Step 1. Calculate the animated tab_width

    float target_width = view.width / state.tab_names.size();
    if (state.initial_tab_width != target_width) {
        auto animation_id = (void*)&state.initial_tab_width;
        if (!animation::is_animating(animation_id)) {
            animation::start(animation_id);
        }
        auto elapsed = animation::get_time_elapsed(animation_id);
        if (elapsed > ANIMATION_DURATION) {
            animation::stop(animation_id);
            tab_width = target_width;
            state.initial_tab_width = tab_width;
        } else {
            auto ratio = animation::ease_out(elapsed / ANIMATION_DURATION);
            tab_width = state.initial_tab_width + (target_width - state.initial_tab_width) * ratio;
        }
    } else {
        tab_width = target_width;
    }

    // Step 2. Calculate the target positions

    float target_positions[state.tab_names.size()];

    if (state.state == TAB_PRESSED) {

        // Calculate the target positions
        for (int i = 0; i < state.tab_names.size(); ++i) {
            target_positions[i] = i;
        }

        int target_i = tab_drag_target_position();

        // Dragging the tab to the left pushes other tabs to the right
        if (target_i < state.active_tab) {
            for (int i = target_i; i < state.active_tab; ++i) {
                target_positions[i]++;
            }
        }

        // Dragging the tab to the right pushes other tabs to the right
        if (target_i > state.active_tab) {
            for (int i = state.active_tab + 1; i <= target_i; ++i) {
                target_positions[i]--;
            }
        }

    } else {

        // When not dragging, the tabs are just next to each other
        for (int i = 0; i < state.tab_names.size(); ++i) {
            target_positions[i] = i;
        }

    }

    // Step 3. Smoothly animate tab xs from current position to target position

    for (int i = 0; i < state.tab_names.size(); ++i) {
        if (state.state == TAB_PRESSED && i == state.active_tab) {
            // Don't animate the dragged tab, user is in control of its position
            float mx, my, dx, dy;
            mouse_movement(&mx, &my, &dx, &dy);
            tab_xs[i] = state.active_tab * tab_width + dx;
            continue;
        }
        if (state.initial_tab_positions[i] != target_positions[i]) {
            auto animation_id = (void*)&state.initial_tab_positions[i];
            if (!animation::is_animating(animation_id)) {
                animation::start(animation_id);
            }
            auto elapsed = animation::get_time_elapsed(animation_id);
            if (elapsed > ANIMATION_DURATION) {
                animation::stop(animation_id);
                state.initial_tab_positions[i] = target_positions[i];
                tab_xs[i] = target_positions[i] * tab_width;
            } else {
                auto ratio = animation::ease_out(elapsed / ANIMATION_DURATION);
                auto pos = state.initial_tab_positions[i] + (target_positions[i] - state.initial_tab_positions[i]) * ratio;
                tab_xs[i] = pos * tab_width;
            }
        } else {
            tab_xs[i] = target_positions[i] * tab_width;
        }
    }
}

void TabBar::draw() {
    ddui::font_face(this->font_face);
    ddui::font_size(this->font_size);

    // Render the plus button
    {
        bool active = false;
        if (state.state == IDLE && mouse_over(new_tab_button_x, 0, new_tab_button_width, view.height)) {
            active = true;
            set_cursor(CURSOR_POINTING_HAND);
        }
        render_button(new_tab_button_x, new_tab_button_width, "+", active, false);
    }
    
    save();
    clip(0, 0, view.width, view.height);

    // Render all non-active tabs
    for (int i = 0; i < state.tab_names.size(); ++i) {
        if (i == state.active_tab) {
            continue;
        }
        bool hovering = state.state == IDLE && mouse_over(tab_xs[i], 0, tab_width, view.height);
        render_button(tab_xs[i], tab_width, state.tab_names[i].c_str(), hovering, false);
    }

    // Render the active tab
    if (state.active_tab != -1) {
        int i = state.active_tab;
        bool hovering = state.state != IDLE || mouse_over(tab_xs[i], 0, tab_width, view.height);
        auto name = state.state == TAB_RENAMING ? "" : state.tab_names[i].c_str();
        render_button(tab_xs[i], tab_width, name, hovering, true);
    }

    restore();

    // Render the renaming text box
    if (state.state == TAB_RENAMING) {
        sub_view(state.active_tab * tab_width, 0, tab_width, view.height);
        PlainTextBox::update(&state.text_box_state);
        restore();
    }
}

void TabBar::open_rename_input() {
    state.state = TAB_RENAMING;
    state.text_box_model.regular_font = this->font_face;
    TextEdit::set_text_content(&state.text_box_model, state.tab_names[state.active_tab].c_str());
    TextEdit::set_style(&state.text_box_model, false, this->font_size, color_text_active);
    state.text_box_state.model = &state.text_box_model;
    state.text_box_state.bg_color = rgba(0x000000, 0.0);
    state.text_box_state.bg_color_focused = rgba(0x000000, 0.0);
    state.text_box_state.border_color = rgba(0x000000, 0.0);
    state.text_box_state.border_color_focused = rgba(0x000000, 0.0);
    state.text_box_state.border_radius = 0;
    state.text_box_state.border_width = 0;
    focus(&state.text_box_state);
}

void TabBar::open_context_menu() {
    float mx, my;
    mouse_position(&mx, &my);

    MenuBuilder mb;
    
    ContextMenu::show(this, mx, my,
        mb.menu()
            .item("Rename").action(0)
            .item("Duplicate").action(1)
            .item("Close").action(state.tab_names.size() > 1 ? 2 : -1)
    );
}

int TabBar::tab_drag_target_position() {
    float mx, my;
    mouse_position(&mx, &my);
    for (int i = 0; i < state.tab_names.size(); ++i) {
        if (mx >= i * tab_width && mx < (i + 1) * tab_width) {
            return i;
        }
    }
    if (mx < 1) {
        return 0;
    } else {
        return state.tab_names.size() - 1;
    }
}

void TabBar::tab_drag_drop() {
    auto reorder_to_index = tab_drag_target_position();

    // Create the action
    if (state.active_tab != reorder_to_index) {
        action.type = REORDER_TAB;
        action.tab_index = state.active_tab;
        action.reorder_to_index = reorder_to_index;
    }

    // Set the initial_tab_position of the active tab to start
    // from where it was dropped
    {
        float mx, my, dx, dy;
        mouse_movement(&mx, &my, &dx, &dy);
        float tab_x = state.active_tab * tab_width + dx;
        state.initial_tab_positions[state.active_tab] = tab_x / tab_width;
    }

    // Reorder the tabs
    if (state.active_tab > reorder_to_index) {
        for (int i = state.active_tab; i > reorder_to_index; --i) {
            std::swap(state.tab_names[i], state.tab_names[i - 1]);
            std::swap(state.initial_tab_positions[i], state.initial_tab_positions[i - 1]);
        }
    }
    if (state.active_tab < reorder_to_index) {
        for (int i = state.active_tab; i < reorder_to_index; ++i) {
            std::swap(state.tab_names[i], state.tab_names[i + 1]);
            std::swap(state.initial_tab_positions[i], state.initial_tab_positions[i + 1]);
        }
    }
    
    // Stop all the animations
    for (int i = 0; i < state.tab_names.size(); ++i) {
        animation::stop(&state.initial_tab_positions[i]);
    }

    // Update the active tab
    state.active_tab = reorder_to_index;
}

int TabBar::find_removal(const std::vector<std::string>& a, const std::vector<std::string>& b) {
    // In the case that exactly one tab's been removed, let's locate it

    if (a.size() - 1 != b.size()) {
        return -1; // Not what we're looking for
    }

    for (int i = 0; i < b.size(); ++i) {
        if (a[i] != b[i]) {
            return i; // i has been removed
        }
    }

    return b.size();
}

void TabBar::render_button(float x, float width, const char* name, bool hovering, bool active) {

    if (hovering) {
        begin_path();
        fill_color(color_bg_hover);
        rect(x, 0, width, view.height);
        fill();
    }

    if (active) {
        fill_color(color_text_active);
    } else if (hovering) {
        fill_color(color_text_hover);
    } else {
        fill_color(color_text);
    }

    draw_centered_text_in_box(x, 0, width, view.height, name);

}
