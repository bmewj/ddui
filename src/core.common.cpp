//
//  core.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 30/08/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "core.hpp"
#include "init.hpp"
#include "timer.hpp"
#include "animation.hpp"
#include <vector>
#include <chrono>
#include <mutex>

namespace ddui {

struct FocusState {
    void* focus_old;
    void* focus_new;
    std::vector<void*> groups;

    enum Action {
        NO_CHANGE,
        TAB_FORWARD,
        TAB_BACKWARD,
        TAB_TO,
        BLUR
    };

    Action action;
    void* tab_to;
};

// Globals
static FocusState focus_state;
static std::vector<KeyState> key_state_queue;
static std::mutex repaint_mutex;
static bool is_painting, should_repaint;
static std::mutex set_immediate_mutex;
static std::vector<std::function<void()>> set_immediate_callbacks;
static Cursor cursor_state_old, cursor_state_new;
MouseState mouse_state;
KeyState key_state;

// Setup
bool init_common() {

    focus_state.focus_old = NULL;
    focus_state.focus_new = NULL;

    mouse_state = { 0 };
    key_state = { 0 };
    cursor_state_old = CURSOR_ARROW;
    cursor_state_new = CURSOR_ARROW;
  
    create_font("entypo", "assets/Entypo.ttf");
    timer_init();

    return true;
}

// User input
void input_key(int key, int scancode, int action, int mods) {
    KeyState key_state;
    key_state.action = action;
    key_state.key = key;
    key_state.mods = mods;
    key_state.character = NULL;
    key_state_queue.push_back(key_state);
}

void input_character(unsigned int codepoint) {
    auto cp = codepoint;

    int n = 0;
    if (cp < 0x80) n = 1;
    else if (cp < 0x800) n = 2;
    else if (cp < 0x10000) n = 3;
    else if (cp < 0x200000) n = 4;
    else if (cp < 0x4000000) n = 5;
    else if (cp <= 0x7fffffff) n = 6;

    auto key_character = new char[7];
    key_character[n] = '\0';

    switch (n) {
        case 6: key_character[5] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x4000000;
        case 5: key_character[4] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x200000;
        case 4: key_character[3] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x10000;
        case 3: key_character[2] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x800;
        case 2: key_character[1] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0xc0;
        case 1: key_character[0] = cp;
    }

    if (key_state_queue.empty()) {
        KeyState key_state;
        key_state.action = keyboard::ACTION_PRESS;
        key_state.key = 0;
        key_state.mods = 0;
        key_state.character = key_character;
        key_state_queue.push_back(key_state);
    } else {
        key_state_queue.back().key = 0;
        key_state_queue.back().character = key_character;
    }
}

void input_mouse_position(float x, float y) {
    mouse_state.x = x;
    mouse_state.y = y;
}

void input_mouse_button(int button, int action, int mods) {
    constexpr int ACTION_RELEASE = 0;
    constexpr int ACTION_PRESS = 1;
    constexpr int MOUSE_BUTTON_LEFT = 0;

    if (action == ACTION_PRESS) {
        mouse_state.accepted = false;
        mouse_state.pressed = false;
        mouse_state.pressed_secondary = false;
        mouse_state.initial_x = mouse_state.x;
        mouse_state.initial_y = mouse_state.y;

        if (button == MOUSE_BUTTON_LEFT) {
            mouse_state.pressed = true;
        } else {
            mouse_state.pressed_secondary = true;
        }
    }

    if (action == ACTION_RELEASE) {
        mouse_state.accepted = false;
        mouse_state.pressed = false;
        mouse_state.pressed_secondary = false;
    }
}

void input_scroll(float offset_x, float offset_y) {
    mouse_state.scroll_dx = offset_x;
    mouse_state.scroll_dy = offset_y;
}

// Frame management
static void update_pre(float width, float height, float pixel_ratio);
static void update_post();

void update(float width, float height, float pixel_ratio, std::function<void()> update_proc) {

    // Setup GL frame
    auto frame_buffer_width  = (int)(width * pixel_ratio);
    auto frame_buffer_height = (int)(height * pixel_ratio);
    glViewport(0, 0, frame_buffer_width, frame_buffer_height);
    glClearColor(0.949f, 0.949f, 0.949f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    repaint_mutex.lock();
    is_painting = true;
    should_repaint = false;
    repaint_mutex.unlock();

    while (true) {
        nvgBeginFrame(vg, width, height, pixel_ratio);
        update_pre(width, height, pixel_ratio);
        update_proc();
        update_post();

        repaint_mutex.lock();
        if (!should_repaint) {
            is_painting = false;
            repaint_mutex.unlock();
            break;
        }
        should_repaint = false;
        repaint_mutex.unlock();

        nvgCancelFrame(vg);
    }

    nvgEndFrame(vg);

}

void update_pre(float width, float height, float pixel_ratio) {

    // Process all set_immediate callbacks
    set_immediate_mutex.lock();
    auto callbacks = std::move(set_immediate_callbacks);
    set_immediate_mutex.unlock();
    for (auto& callback : callbacks) {
        callback();
    }

    // Let the animation system know that a new frame is being generated
    update_animation();

    cursor_state_new = CURSOR_ARROW;

    if (!key_state_queue.empty()) {
        key_state = key_state_queue.front();
        key_state_queue.erase(key_state_queue.begin());
    } else {
        key_state.action = 0;
        if (key_state.character) {
            delete[] key_state.character;
            key_state.character = NULL;    
        }
        key_state.key = 0;
    }

    focus_state.action = FocusState::NO_CHANGE;
    focus_state.groups.clear();

    view.width = width;
    view.height = height;
    view.clip.x1 = 0.0;
    view.clip.y1 = 0.0;
    view.clip.x2 = width;
    view.clip.y2 = height;
    saved_views.clear();
    saved_views.push_back(view);
}

void update_post() {

    mouse_state.scroll_dx = 0.0;
    mouse_state.scroll_dy = 0.0;

    if (key_state.action == keyboard::ACTION_RELEASE) {
        if (key_state.key == keyboard::KEY_TAB) {
            if (key_state.mods & keyboard::MOD_SHIFT) {
                tab_backward();
            } else {
                tab_forward();
            }
            consume_key_event();
        }
    }
    
    focus_state.focus_old = focus_state.focus_new;
    focus_state.focus_new = NULL;
    
    int group_index = -1;
    for (int i = 0; i < focus_state.groups.size(); ++i) {
        if (focus_state.groups[i] == focus_state.focus_old) {
            group_index = i;
            break;
        }
    }
    
    switch (focus_state.action) {
        case FocusState::NO_CHANGE:
            if (group_index != -1) {
                focus_state.focus_new = focus_state.focus_old;
            }
            break;
        case FocusState::TAB_FORWARD:
            if (group_index != -1 && group_index + 1 < focus_state.groups.size()) {
                focus_state.focus_new = focus_state.groups[group_index + 1];
            } else if (focus_state.focus_old == NULL && !focus_state.groups.empty()) {
                focus_state.focus_new = focus_state.groups.front();
            }
            break;
        case FocusState::TAB_BACKWARD:
            if (group_index != -1 && group_index - 1 >= 0) {
                focus_state.focus_new = focus_state.groups[group_index - 1];
            } else if (focus_state.focus_old == NULL && !focus_state.groups.empty()) {
                focus_state.focus_new = focus_state.groups.back();
            }
            break;
        case FocusState::TAB_TO:
            for (int i = 0; i < focus_state.groups.size(); ++i) {
                if (focus_state.groups[i] == focus_state.tab_to) {
                    focus_state.focus_new = focus_state.tab_to;
                    break;
                }
            }
            break;
        case FocusState::BLUR:
            break;
    }
    
    if (focus_state.focus_old != focus_state.focus_new ||
        !key_state_queue.empty()) {
        repaint();
    }

    if (cursor_state_old != cursor_state_new) {
        cursor_state_old  = cursor_state_new;
        if (set_cursor_proc) {
            set_cursor_proc(cursor_state_new);
        }
    }
}

void repaint() {
    repaint_mutex.lock();
    if (is_painting) {
        should_repaint = true;
    } else {
        if (post_empty_message_proc) {
            post_empty_message_proc();
        } else {
            printf("post_empty_message_proc not set! This is a crucial proc for ddui.\n");
        }
    }
    repaint_mutex.unlock();
}

void set_immediate(std::function<void()> callback) {
    set_immediate_mutex.lock();
    set_immediate_callbacks.push_back(std::move(callback));
    set_immediate_mutex.unlock();
    repaint();
}

// Color utils
Color rgb(unsigned char r, unsigned char g, unsigned char b) {
    Color color;
    color.r = r / (float)0xff;
    color.g = g / (float)0xff;
    color.b = b / (float)0xff;
    color.a = 1.0;
    return color;
}
Color rgb(unsigned int rgb) {
    Color color;
    color.r = ((rgb >> 16) & 0xff) / (float)0xff;
    color.g = ((rgb >>  8) & 0xff) / (float)0xff;
    color.b = ((rgb >>  0) & 0xff) / (float)0xff;
    color.a = 1.0;
    return color;
}
Color rgba(unsigned char r, unsigned char g, unsigned char b, float a) {
    Color color;
    color.r = r / (float)0xff;
    color.g = g / (float)0xff;
    color.b = b / (float)0xff;
    color.a = a;
    return color;
}
Color rgba(unsigned int rgb, float a) {
    Color color;
    color.r = ((rgb >> 16) & 0xff) / (float)0xff;
    color.g = ((rgb >>  8) & 0xff) / (float)0xff;
    color.b = ((rgb >>  0) & 0xff) / (float)0xff;
    color.a = a;
    return color;
}
Color color_interpolate(Color a, Color b, float ratio) {
    Color color;
    color.r = a.r + ratio * (b.r - a.r);
    color.g = a.g + ratio * (b.g - a.g);
    color.b = a.b + ratio * (b.b - a.b);
    color.a = a.a + ratio * (b.a - a.a);
    return color;
}

bool mouse_hit(float x, float y, float width, float height) {
    return (
        !mouse_state.accepted && mouse_state.pressed &&
        mouse_inside(x, y, width, height)
    );
}

bool mouse_hit_secondary(float x, float y, float width, float height) {
    return (
        !mouse_state.accepted && mouse_state.pressed_secondary &&
        mouse_inside(x, y, width, height)
    );
}

bool mouse_over(float x, float y, float width, float height) {
    return (
        !mouse_state.accepted && !mouse_state.pressed &&
        mouse_inside(x, y, width, height)
    );
}

void mouse_hit_accept() {
    mouse_state.accepted = true;
}

// Focus state
void register_focus_group(void* identifier) {
    focus_state.groups.push_back(identifier);
}

bool did_focus(void* identifier) {
    return (focus_state.focus_old != identifier &&
            focus_state.focus_new == identifier);
}

bool did_blur(void* identifier) {
    return (focus_state.focus_old == identifier &&
            focus_state.focus_new != identifier);
}

bool has_focus(void* identifier) {
    return (focus_state.focus_new == identifier);
}

void tab_forward() {
    focus_state.action = FocusState::TAB_FORWARD;
}

void tab_backward() {
    focus_state.action = FocusState::TAB_BACKWARD;
}

void focus(void* identifier) {
    focus_state.action = FocusState::TAB_TO;
    focus_state.tab_to = identifier;
}

void blur() {
    focus_state.action = FocusState::BLUR;
}

// Keyboard state
bool has_key_event() {
    return (key_state.character != NULL || key_state.key > 0);
}

bool has_key_event(void* identifier) {
    return (focus_state.focus_new == identifier) && (key_state.character != NULL || key_state.key > 0);
}

void consume_key_event() {
    key_state = { 0 };
}

void repeat_key_event() {
    key_state_queue.insert(key_state_queue.begin(), key_state);
    key_state = { 0 };
}

const char* get_clipboard_string() {
    if (get_clipboard_string_proc) {
        return get_clipboard_string_proc();
    } else {
        return NULL;
    }
}

void set_clipboard_string(const char* string) {
    if (set_clipboard_string_proc) {
        set_clipboard_string_proc(string);
    }
}

// Cursor state
void set_cursor(Cursor cursor) {
    cursor_state_new = cursor;
}

}
