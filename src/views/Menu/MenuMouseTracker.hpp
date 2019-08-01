//
//  MenuMouseTracker.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 01/08/2019.
//  Copyright © 2019 Bartholomew Joyce All rights reserved.
//

#ifndef MenuMouseTracker_hpp
#define MenuMouseTracker_hpp

#include <ddui/core>

struct MenuMouseTracker {

    struct State {
        bool did_init = false;
        bool is_first_press;
        bool is_pressing;
        int first_press_x, first_press_y;
        int first_press_travelled;
    };

    inline MenuMouseTracker(State& state);
    inline void update();

    bool did_press;
    bool did_release;

private:
    State& state;
};

MenuMouseTracker::MenuMouseTracker(State& state) : state(state) {}

void MenuMouseTracker::update() {

    constexpr int MIN_TRAVEL_DISTANCE = 5;

    // We don't care about clicks being primary or secondary
    bool pressed = (ddui::mouse_state.pressed || ddui::mouse_state.pressed_secondary);

    // But we do care whether we're inside the viewport
    bool inside_viewport;
    {
        float mx, my;
        ddui::mouse_position(&mx, &my);
        inside_viewport = (
            (0.0 <= mx && mx < ddui::view.width) && (0.0 <= my && my < ddui::view.height)
        );
    }

    // Initialise state!
    // This is only used when the mouse is pressed at the point of initialising.
    // This is referred to as the first press, and it has to be handled carefully.
    // If the user releases the first press, it should be considered an actual
    // release only if the user clicked and dragged (i.e. travelled some distance
    // between pressing and releasing).
    if (!state.did_init) {
        state.did_init = true;
        state.is_pressing = pressed;
        state.is_first_press = pressed;
        state.first_press_x = ddui::mouse_state.x;
        state.first_press_y = ddui::mouse_state.y;
        state.first_press_travelled = 0;
    }

    // Update the first press travel distance
    if (pressed && state.is_first_press) {
        int dx = ddui::mouse_state.x - state.first_press_x;
        int dy = ddui::mouse_state.y - state.first_press_y;
        if (dx < 0) dx = -dx;
        if (dy < 0) dy = -dy;
        state.first_press_travelled += dx + dy;
        state.first_press_x = ddui::mouse_state.x;
        state.first_press_y = ddui::mouse_state.y;
    }

    // Calculate state transitions
    did_press = (!state.is_pressing && pressed && inside_viewport);
    did_release = (state.is_pressing && !pressed);

    // Correctly handle first_press release
    if (did_release && state.is_first_press) {
        if (state.first_press_travelled < MIN_TRAVEL_DISTANCE) {
            did_release = false;
        }
        state.is_first_press = false;
    }

    // Update state
    state.is_pressing = pressed;
}

#endif
