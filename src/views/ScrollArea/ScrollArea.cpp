//
//  ScrollArea.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 11/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "ScrollArea.hpp"

namespace ScrollArea {

using namespace ddui;

constexpr int BAR_WIDTH = 10;

#define LIMIT(l, x, u)\
    if (x > u) x = u;\
    if (x < l) x = l;

ScrollAreaState::ScrollAreaState() {
    scroll_x = 0;
    scroll_y = 0;
    initial_scroll_x = 0;
    initial_scroll_y = 0;
    is_dragging_horizontal_bar = false;
    is_dragging_vertical_bar = false;
    scroll_into_view.requested = false;
}

void update(ScrollAreaState* state, float inner_width, float inner_height, std::function<void()> update_inner) {

    auto container_width = view.width;
    auto container_height = view.height;

    float mx, my, dx, dy;
    mouse_movement(&mx, &my, &dx, &dy);

    // Update the scroll position
    if (mouse_over(0, 0, view.width, view.height)) {
        state->scroll_x += mouse_state.scroll_dx;
        state->scroll_y += mouse_state.scroll_dy;
    }
    if (mouse_state.pressed && state->is_dragging_horizontal_bar) {
        state->scroll_x = state->initial_scroll_x + (dx * inner_width) / container_width;
    }
    if (mouse_state.pressed && state->is_dragging_vertical_bar) {
        state->scroll_y = state->initial_scroll_y + (dy * inner_height) / container_height;
    }

    // Scroll into view
    if (state->scroll_into_view.requested) {
        auto x1 = state->scroll_into_view.x;
        auto y1 = state->scroll_into_view.y;
        auto x2 = x1 + state->scroll_into_view.width;
        auto y2 = y1 + state->scroll_into_view.height;

        LIMIT(x2 - container_width,  state->scroll_x, x1);
        LIMIT(y2 - container_height, state->scroll_y, y1);

        state->scroll_into_view.requested = false;
    }

    // Prevent scrolling past frame boundaries
    LIMIT(0, state->scroll_x, inner_width - container_width);
    LIMIT(0, state->scroll_y, inner_height - container_height);

    // Division by 0 failsafe
    if (inner_width == 0) {
        inner_width = 1;
    }
    if (inner_height == 0) {
        inner_height = 1;
    }

    // Calculate scrollbar sizes
    bool show_h_bar = (container_width < inner_width);
    int h_bar_x = (state->scroll_x * container_width) / inner_width;
    int h_bar_y = container_height - BAR_WIDTH;
    int h_bar_w = (container_width * container_width) / inner_width;
    int h_bar_h = BAR_WIDTH;
  
    bool show_v_bar = (container_height < inner_height);
    int v_bar_x = container_width - BAR_WIDTH;
    int v_bar_y = (state->scroll_y * container_height) / inner_height;
    int v_bar_w = BAR_WIDTH;
    int v_bar_h = (container_height * container_height) / inner_height;
  
    // Scroll bar hovering/dragging
    if (show_h_bar && mouse_hit(h_bar_x, h_bar_y, h_bar_w, h_bar_h)) {
        mouse_hit_accept();
        state->is_dragging_horizontal_bar = true;
        state->initial_scroll_x = state->scroll_x;
    }
    if (state->is_dragging_horizontal_bar && !mouse_state.pressed) {
        state->is_dragging_horizontal_bar = false;
    }
  
    if (show_v_bar && mouse_hit(v_bar_x, v_bar_y, v_bar_w, v_bar_h)) {
        mouse_hit_accept();
        state->is_dragging_vertical_bar = true;
        state->initial_scroll_y = state->scroll_y;
    }
    if (state->is_dragging_vertical_bar && !mouse_state.pressed) {
        state->is_dragging_vertical_bar = false;
    }
  
    // Draw the content
    save();
    clip(0, 0, container_width, container_height);
    sub_view(-state->scroll_x, -state->scroll_y,
             container_width  > inner_width  ? container_width  : inner_width,
             container_height > inner_height ? container_height : inner_height);
    update_inner();
    restore();
    restore();
  
    // Draw scroll bars
    if (show_h_bar) {
        auto color = rgba(0x000000, 0.3);
        if (state->is_dragging_horizontal_bar) {
            color = rgba(0x2a9fd6, 0.8);
        } else if (mouse_over(h_bar_x, h_bar_y, h_bar_w, h_bar_h)) {
            color = rgba(0x646464, 0.8);
        }
        begin_path();
        fill_color(color);
        rect(h_bar_x, h_bar_y, h_bar_w, h_bar_h);
        fill();
    }
    if (show_v_bar) {
        auto color = rgba(0x000000, 0.3);
        if (state->is_dragging_vertical_bar) {
            color = rgba(0x2a9fd6, 0.8);
        } else if (mouse_over(v_bar_x, v_bar_y, v_bar_w, v_bar_h)) {
            color = rgba(0x646464, 0.8);
        }
        begin_path();
        fill_color(color);
        rect(v_bar_x, v_bar_y, v_bar_w, v_bar_h);
        fill();
    }
}

void scroll_into_view(ScrollAreaState* state, float x, float y, float width, float height) {
    state->scroll_into_view.requested = true;
    state->scroll_into_view.x = x;
    state->scroll_into_view.y = y;
    state->scroll_into_view.width = width;
    state->scroll_into_view.height = height;
    repaint("ScrollArea::scroll_into_view");
}

}
