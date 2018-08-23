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

using namespace ddui;

struct OverlayState {
    void* identifier = NULL;
    bool active;
    std::function<void()> inner_update;
};

static std::vector<OverlayState> overlay_stack;
static MouseState empty_mouse = { 0 };
static KeyState empty_key = { 0 };

void update(std::function<void()> inner_update) {

    // All overlays start out inactive (to detect closes of overlays)
    for (auto& overlay : overlay_stack) {
        overlay.active = false;
    }

    // Block user input to background content when an overlay is open
    auto original_mouse_state = mouse_state;
    auto original_key_state = key_state;
    if (!overlay_stack.empty()) {
        mouse_state = { 0 };
        key_state = { 0 };
    }

    // Save current identifiers to compare
    auto old_size = overlay_stack.size();
    void* old_identifiers[old_size];
    for (int i = 0; i < overlay_stack.size(); ++i) {
        old_identifiers[i] = overlay_stack[i].identifier;
    }
    
    // Update background content
    inner_update();
    
    // Draw all the overlays in order
    for (int i = 0; i < overlay_stack.size(); ++i) {
        auto& overlay = overlay_stack[i];
        
        // The overlay has changed, repaint
        if (i >= old_size || overlay.identifier != old_identifiers[i]) {
            post_empty_message();
            return;
        }
        
        // The overlay hasn't got a handler, so close it
        if (!overlay.active) {
            overlay_stack.erase(overlay_stack.begin() + i, overlay_stack.end());
            post_empty_message();
            return;
        }
        
        // Draw the overlay
        auto is_top_most_overlay = (i == overlay_stack.size() - 1);
        if (is_top_most_overlay) {
            mouse_state = original_mouse_state;
            key_state = original_key_state;
        }
        overlay.inner_update();
        overlay.inner_update = std::function<void()>();
        
    }

    // Unhandled mouse clicks trigger overlay close
    if (!overlay_stack.empty() && mouse_hit(0, 0, view.width, view.height)) {
        mouse_hit_accept();

        overlay_stack.pop_back();
        post_empty_message();
    }

}

void handle_overlay(void* identifier, std::function<void()> inner_update) {

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
