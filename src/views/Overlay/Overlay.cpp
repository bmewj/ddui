//
//  Overlay.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 10/07/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "Overlay.hpp"

namespace Overlay {

struct OverlayState {
    void* identifier = NULL;
    int bounds[4];
    bool active;
    std::function<void(Context)> inner_update;
};

static OverlayState state;
static MouseState empty_mouse = { 0 };
static FocusState empty_focus = { 0 };
static KeyState empty_key = { 0 };

void update(Context ctx, std::function<void(Context)> inner_update) {

    state.active = false;

    // Block user input to background content when an overlay is open
    auto child_ctx = ctx;
    if (state.identifier != NULL) {
        child_ctx.mouse = &empty_mouse;
        child_ctx.focus = &empty_focus;
        child_ctx.key = &empty_key;
    }

    // Update background content
    auto old_identifier = state.identifier;
    inner_update(child_ctx);

    // The overlay has changed, repaint
    if (state.identifier != old_identifier) {
        *ctx.must_repaint = true;
        return;
    }

    // Overlay hasn't changed AND is null, so do nothing
    if (state.identifier == NULL) {
        return;
    }

    // Overlay is open, but hasn't got a handler, so close it
    if (!state.active) {
        state.identifier = NULL;
        *ctx.must_repaint = true;
        return;
    }

    // Draw overlay
    state.inner_update(ctx);
    state.inner_update = std::function<void(Context)>();

    // Unhandled mouse clicks trigger overlay close
    if (mouse_hit(ctx, 0, 0, ctx.width, ctx.height)) {
        ctx.mouse->accepted = true;

        state.identifier = NULL;
        *ctx.must_repaint = true;
    }

}

void handle_overlay(void* identifier, std::function<void(Context)> inner_update) {

    if (state.identifier == identifier) {
        state.active = true;
        state.inner_update = std::move(inner_update);
    }

}

void open(void* identifier) {

    if (state.identifier != identifier) {
        state.identifier = identifier;
        state.active = false;
    }

}

void close(void* identifier) {

    if (state.identifier == identifier) {
        state.identifier = NULL;
    }

}

bool is_open(void* identifier) {
    return state.identifier == identifier;
}

}
