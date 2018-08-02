//
//  Overlay.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 10/07/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "Overlay.hpp"
#include <vector>

namespace Overlay {

struct OverlayState {
    void* identifier = NULL;
    int bounds[4];
    bool active;
    std::function<void(Context)> inner_update;
};

static std::vector<OverlayState> overlay_stack;
static MouseState empty_mouse = { 0 };
static FocusState empty_focus = { 0 };
static KeyState empty_key = { 0 };

void update(Context ctx, std::function<void(Context)> inner_update) {

    // All overlays start out inactive (to detect closes of overlays)
    for (auto& overlay : overlay_stack) {
        overlay.active = false;
    }

    // Block user input to background content when an overlay is open
    auto child_ctx = ctx;
    if (!overlay_stack.empty()) {
        child_ctx.mouse = &empty_mouse;
        child_ctx.focus = &empty_focus;
        child_ctx.key = &empty_key;
    }

    // Save current identifiers to compare
    auto old_size = overlay_stack.size();
    void* old_identifiers[old_size];
    for (int i = 0; i < overlay_stack.size(); ++i) {
        old_identifiers[i] = overlay_stack[i].identifier;
    }
    
    // Update background content
    inner_update(child_ctx);
    
    // Draw all the overlays in order
    for (int i = 0; i < overlay_stack.size(); ++i) {
        auto& overlay = overlay_stack[i];
        
        // The overlay has changed, repaint
        if (i >= old_size || overlay.identifier != old_identifiers[i]) {
            *ctx.must_repaint = true;
            return;
        }
        
        // The overlay hasn't got a handler, so close it
        if (!overlay.active) {
            overlay_stack.erase(overlay_stack.begin() + i, overlay_stack.end());
            *ctx.must_repaint = true;
            return;
        }
        
        // Draw the overlay
        auto is_top_most_overlay = (i == overlay_stack.size() - 1);
        overlay.inner_update(is_top_most_overlay ? ctx : child_ctx);
        overlay.inner_update = std::function<void(Context)>();
        
    }

    // Unhandled mouse clicks trigger overlay close
    if (!overlay_stack.empty() && mouse_hit(ctx, 0, 0, ctx.width, ctx.height)) {
        ctx.mouse->accepted = true;

        overlay_stack.pop_back();
        *ctx.must_repaint = true;
    }

}

void handle_overlay(void* identifier, std::function<void(Context)> inner_update) {

    for (auto& overlay : overlay_stack) {
        if (overlay.identifier == identifier) {
            overlay.active = true;
            overlay.inner_update = std::move(inner_update);
            break;
        }
    }

}

void open(void* identifier) {

    OverlayState overlay;
    overlay.identifier = identifier;
    overlay.active = false;
    overlay_stack.push_back(overlay);

}

void close(void* identifier) {

    for (int i = 0; i < overlay_stack.size(); ++i) {
        if (overlay_stack[i].identifier == identifier) {
            overlay_stack.erase(overlay_stack.begin() + i, overlay_stack.end());
            return;
        }
    }

}

bool is_open(void* identifier) {
    for (auto& overlay : overlay_stack) {
        if (overlay.identifier == identifier) {
            return true;
        }
    }
    return false;
}

}
