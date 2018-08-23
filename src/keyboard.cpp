//
//  keyboard.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 14/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "keyboard.hpp"

namespace keyboard {

void register_focus_group(Context ctx, void* identifier) {
    ctx.focus->groups.push_back(identifier);
}

bool did_focus(Context ctx, void* identifier) {
    return (ctx.focus->focus_old != identifier &&
            ctx.focus->focus_new == identifier);
}

bool did_blur(Context ctx, void* identifier) {
    return (ctx.focus->focus_old == identifier &&
            ctx.focus->focus_new != identifier);
}

bool has_focus(Context ctx, void* identifier) {
    return (ctx.focus->focus_new == identifier);
}

void tab_forward(Context ctx) {
    ctx.focus->action = FocusState::TAB_FORWARD;
}

void tab_backward(Context ctx) {
    ctx.focus->action = FocusState::TAB_BACKWARD;
}

void focus(Context ctx, void* identifier) {
    ctx.focus->action = FocusState::TAB_TO;
    ctx.focus->tab_to = identifier;
}

void blur(Context ctx) {
    ctx.focus->action = FocusState::BLUR;
}

bool has_key_event(Context ctx) {
    return (key_state.character != NULL || key_state.key > 0);
}

bool has_key_event(Context ctx, void* identifier) {
    return (ctx.focus->focus_new == identifier) && (key_state.character != NULL || key_state.key > 0);
}

void consume_key_event(Context ctx) {
    *ctx.key = { 0 };
}

}
