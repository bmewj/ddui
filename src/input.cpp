//
//  input.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 01/10/2019.
//  Copyright © 2019 Bartholomew Joyce All rights reserved.
//

#include "core.hpp"
#include <deque>
#include <cstring>

namespace ddui {

// We may receive multiple events simultaneously, and the job of input.cpp
// is to decode them appropriately and then feed them into the user code
// in a simple manner. If we received multiple character events, we want
// to call an update() for each character in order, so that the user code
// can process each character properly in sequence.

constexpr int ACTION_RELEASE = 0;
constexpr int ACTION_PRESS = 1;
constexpr int ACTION_REPEAT = 2;
constexpr int KEY_SHIFT = 340;
constexpr int KEY_CONTROL = 341;
constexpr int KEY_ALT = 342;
constexpr int KEY_SUPER = 343;

struct InputEvent {

    enum Type {
        MOUSE_POSITION,
        MOUSE_PRESS,
        MOUSE_PRESS_SECONDARY,
        MOUSE_RELEASE,
        MOUSE_RELEASE_SECONDARY,
        MOUSE_SCROLL,
        KEY_PRESS,
        KEY_REPEAT,
        KEY_RELEASE,
    };

    Type type;
    int x, y;
    int key, mods;
    char character[7];

};

static std::deque<InputEvent> input_events_queue;

void input_key(int key, int scancode, int action, int mods) {

    input_events_queue.push_back(InputEvent());
    auto& event = input_events_queue.back();

    if (action == ACTION_RELEASE) event.type = InputEvent::KEY_RELEASE;
    if (action == ACTION_PRESS)   event.type = InputEvent::KEY_PRESS;
    if (action == ACTION_REPEAT)  event.type = InputEvent::KEY_REPEAT;

    event.key = key;

    // Fix the mod keys. On a PRESS or REPEAT we want to include
    // the current key as a mod, and on RELEASE we want to exclude it.
    if (action != ACTION_RELEASE) {
        if (key == KEY_SHIFT)   mods |=  keyboard::MOD_SHIFT;
        if (key == KEY_CONTROL) mods |=  keyboard::MOD_CONTROL;
        if (key == KEY_ALT)     mods |=  keyboard::MOD_ALT;
        if (key == KEY_SUPER)   mods |=  keyboard::MOD_SUPER;
    } else {
        if (key == KEY_SHIFT)   mods &= ~keyboard::MOD_SHIFT;
        if (key == KEY_CONTROL) mods &= ~keyboard::MOD_CONTROL;
        if (key == KEY_ALT)     mods &= ~keyboard::MOD_ALT;
        if (key == KEY_SUPER)   mods &= ~keyboard::MOD_SUPER;
    }

    event.mods = mods;

    event.character[0] = '\0';
}

void input_character(unsigned int codepoint) {

    // input_character() events should follow input_key() PRESS or
    // REPEAT events, because essentially input_character() is just
    // extra information regarding the previous event. As such, I
    // need to lookup the last pushed event, and glue on the character
    // data.

    // Step 1. Find a valid key event to stick our character data
    // onto
    bool has_valid_key_event;
    if (input_events_queue.empty()) {
        has_valid_key_event = false;
    } else {
        const auto& event = input_events_queue.back();
        has_valid_key_event = (
            (event.type == InputEvent::KEY_PRESS ||
             event.type == InputEvent::KEY_REPEAT) &&
            event.character[0] == '\0'
        );
    }

    // Step 2. If we don't have a valid key event, then we should
    // create a new one
    if (!has_valid_key_event) {
        input_events_queue.push_back(InputEvent());
        auto& event = input_events_queue.back();
        event.type = InputEvent::KEY_PRESS;
        event.key = 0;
        event.mods = 0;
        event.character[0] = '\0';
    }

    auto& event = input_events_queue.back();

    // Step 3. Convert the codepoint to a UTF-8 string
    {
        unsigned int cp = codepoint;

        int n = 0;
        if (cp < 0x80) n = 1;
        else if (cp < 0x800) n = 2;
        else if (cp < 0x10000) n = 3;
        else if (cp < 0x200000) n = 4;
        else if (cp < 0x4000000) n = 5;
        else if (cp <= 0x7fffffff) n = 6;

        char* key_character = event.character;
        key_character[n] = '\0';

        switch (n) {
            case 6: key_character[5] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x4000000;
            case 5: key_character[4] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x200000;
            case 4: key_character[3] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x10000;
            case 3: key_character[2] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x800;
            case 2: key_character[1] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0xc0;
            case 1: key_character[0] = cp;
        }
    }
}

void input_mouse_position(float x, float y) {
    input_events_queue.push_back(InputEvent());
    auto& event = input_events_queue.back();

    event.type = InputEvent::MOUSE_POSITION;
    event.x = x;
    event.y = y;
}

void input_mouse_button(int button, int action, int mods) {
    input_events_queue.push_back(InputEvent());
    auto& event = input_events_queue.back();

    constexpr int MOUSE_BUTTON_LEFT = 0;

    if (action == ACTION_PRESS) {
        if (button == MOUSE_BUTTON_LEFT) {
            event.type = InputEvent::MOUSE_PRESS;
        } else {
            event.type = InputEvent::MOUSE_PRESS_SECONDARY;
        }
    } else {
        if (button == MOUSE_BUTTON_LEFT) {
            event.type = InputEvent::MOUSE_RELEASE;
        } else {
            event.type = InputEvent::MOUSE_RELEASE_SECONDARY;
        }
    }

    event.mods = mods;
}

void input_scroll(float offset_x, float offset_y) {
    input_events_queue.push_back(InputEvent());
    auto& event = input_events_queue.back();

    event.type = InputEvent::MOUSE_SCROLL;
    event.x = offset_x;
    event.y = offset_y;
}

void consume_key_event() {
    auto saved_mods = key_state.mods;
    key_state = { 0 };
    key_state.mods = saved_mods;
}

void repeat_key_event() {
    if (key_state.action == 0) {
        return;
    }

    input_events_queue.push_front(InputEvent());
    auto& event = input_events_queue.front();

    event.type = (
        key_state.action == keyboard::ACTION_PRESS  ? InputEvent::KEY_PRESS   :
        key_state.action == keyboard::ACTION_REPEAT ? InputEvent::KEY_REPEAT  :
        /* otherwise */                               InputEvent::KEY_RELEASE
    );
    event.key = key_state.key;
    event.mods = key_state.mods;
    if (key_state.character == NULL) {
        event.character[0] = '\0';
    } else {
        std::strcpy(event.character, key_state.character);
    }
}

void pop_input_events_into_global_state() {
    // This function is called once before each update(). It is supposed
    // to place into the global state as many input events as possible, but
    // never: more than one mouse click event simultaneously, more than one
    // distinct key event simultaneously.

    static char character_buffer[8];
    mouse_state.scroll_dx = 0;
    mouse_state.scroll_dy = 0;
    key_state.action = 0;
    key_state.key = 0;
    key_state.character = NULL;

    if (input_events_queue.empty()) {
        return;
    }

    bool popped_mouse_event = false;
    bool popped_key_event = false;

    while (!input_events_queue.empty()) {
        const auto& event = input_events_queue.front();

        // Mouse position events
        if (event.type == InputEvent::MOUSE_POSITION) {
            mouse_state.x = event.x;
            mouse_state.y = event.y;
            input_events_queue.pop_front();
            continue;
        }

        // Mouse scroll events
        if (event.type == InputEvent::MOUSE_SCROLL) {
            mouse_state.scroll_dx += event.x;
            mouse_state.scroll_dy += event.y;
            input_events_queue.pop_front();
            continue;
        }

        // Keyboard input events
        if (event.type == InputEvent::KEY_PRESS ||
            event.type == InputEvent::KEY_REPEAT ||
            event.type == InputEvent::KEY_RELEASE) {

            // Do not pop this event if we've already
            // popped a key input event
            if (popped_key_event) {
                break;
            }

            key_state.action = (
                event.type == InputEvent::KEY_PRESS   ? keyboard::ACTION_PRESS   :
                event.type == InputEvent::KEY_REPEAT  ? keyboard::ACTION_REPEAT  :
                /* otherwise */                         keyboard::ACTION_RELEASE
            );
            key_state.key = event.key;
            key_state.mods = event.mods;
            if (event.character[0] != '\0') {
                std::strcpy(character_buffer, event.character);
                key_state.character = character_buffer;
            }

            input_events_queue.pop_front();
            popped_key_event = true;
            continue;
        }

        // Mouse click events
        {

            // Do not pop this event if we've already
            // popped a mouse click event
            if (popped_mouse_event) {
                break;
            }

            key_state.mods = event.mods;
            if (event.type == InputEvent::MOUSE_PRESS) {
                mouse_state.accepted = false;
                mouse_state.pressed = true;
                mouse_state.pressed_secondary = false;
                mouse_state.initial_x = mouse_state.x;
                mouse_state.initial_y = mouse_state.y;
            } else if (event.type == InputEvent::MOUSE_PRESS_SECONDARY) {
                mouse_state.accepted = false;
                mouse_state.pressed = false;
                mouse_state.pressed_secondary = true;
                mouse_state.initial_x = mouse_state.x;
                mouse_state.initial_y = mouse_state.y;
            } else {
                mouse_state.accepted = false;
                mouse_state.pressed = false;
                mouse_state.pressed_secondary = false;
                mouse_state.initial_x = 0;
                mouse_state.initial_y = 0;
            }

            input_events_queue.pop_front();
            popped_mouse_event = true;
            continue;
        }

    }

}

bool has_input_events_to_process() {
    return !input_events_queue.empty();
}

}
