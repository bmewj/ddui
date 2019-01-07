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
    bool should_close;
    std::function<void()> inner_update;
};

static std::vector<OverlayState> overlay_stack;

void update(std::function<void()> inner_update) {

    // Close overlays that should close
    for (int i = 0; i < overlay_stack.size(); ++i) {
        if (overlay_stack[i].should_close) {
            overlay_stack.erase(overlay_stack.begin() + i, overlay_stack.end());
            break;
        }
    }

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
        mouse_state.x = -1;
        mouse_state.y = -1;
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
            repaint("Overlay::update(1)");
            return;
        }
        
        // The overlay hasn't got a handler, so close it
        if (!overlay.active) {
            overlay_stack.erase(overlay_stack.begin() + i, overlay_stack.end());
            repaint("Overlay::update(2)");
            return;
        }
        
        // Draw the overlay
        auto is_top_most_overlay = (i == overlay_stack.size() - 1);
        if (is_top_most_overlay) {
            mouse_state = original_mouse_state;
            key_state = original_key_state;
        }
        overlay.inner_update();
        
        overlay_stack[i].inner_update = std::function<void()>();
        
    }

    // Unhandled mouse clicks trigger overlay close
    if (!overlay_stack.empty() && mouse_hit(0, 0, view.width, view.height)) {
        mouse_hit_accept();

        overlay_stack.pop_back();
        repaint("Overlay::update(3)");
    }

    // Close overlays that should close
    for (int i = 0; i < overlay_stack.size(); ++i) {
        if (overlay_stack[i].should_close) {
            overlay_stack.erase(overlay_stack.begin() + i, overlay_stack.end());
            repaint("Overlay::update(4)");
            return;
        }
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
    overlay.should_close = false;
    overlay_stack.push_back(overlay);

}

void close(void* identifier) {

    for (int i = 0; i < overlay_stack.size(); ++i) {
        if (overlay_stack[i].identifier == identifier) {
            overlay_stack[i].should_close = true;
            repaint("Overlay::close");
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
