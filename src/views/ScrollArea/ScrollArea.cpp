//
//  ScrollArea.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 11/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "ScrollArea.hpp"
#include <nanovg.h>

namespace ScrollArea {

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

    scroll_into_view.x = -1;
}

void update(ScrollAreaState* state, Context ctx, int inner_width, int inner_height, std::function<void(Context)> update_inner) {

    int container_width = ctx.width;
    int container_height = ctx.height;

    // Update the scroll position
    if (mouse_over(ctx, 0, 0, ctx.width, ctx.height)) {
        state->scroll_x += ctx.mouse->scroll_dx;
        state->scroll_y += ctx.mouse->scroll_dy;
    }
    if (ctx.mouse->pressed && state->is_dragging_horizontal_bar) {
        int dx = ctx.mouse->x - ctx.mouse->initial_x;
        state->scroll_x = state->initial_scroll_x + (dx * inner_width) / container_width;
    }
    if (ctx.mouse->pressed && state->is_dragging_vertical_bar) {
        int dy = ctx.mouse->y - ctx.mouse->initial_y;
        state->scroll_y = state->initial_scroll_y + (dy * inner_height) / container_height;
    }

    // Scroll into view
    if (state->scroll_into_view.x != -1) {
        int x1 = state->scroll_into_view.x;
        int y1 = state->scroll_into_view.y;
        int x2 = x1 + state->scroll_into_view.width;
        int y2 = y1 + state->scroll_into_view.height;

        LIMIT(x2 - container_width,  state->scroll_x, x1);
        LIMIT(y2 - container_height, state->scroll_y, y1);

        state->scroll_into_view.x = -1;
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
    if (show_h_bar && mouse_hit(ctx, h_bar_x, h_bar_y, h_bar_w, h_bar_h)) {
        ctx.mouse->accepted = true;
        state->is_dragging_horizontal_bar = true;
        state->initial_scroll_x = state->scroll_x;
    }
    if (state->is_dragging_horizontal_bar && !ctx.mouse->pressed) {
        state->is_dragging_horizontal_bar = false;
    }
  
    if (show_v_bar && mouse_hit(ctx, v_bar_x, v_bar_y, v_bar_w, v_bar_h)) {
        ctx.mouse->accepted = true;
        state->is_dragging_vertical_bar = true;
        state->initial_scroll_y = state->scroll_y;
    }
    if (state->is_dragging_vertical_bar && !ctx.mouse->pressed) {
        state->is_dragging_vertical_bar = false;
    }
  
    // Draw the content
    nvgSave(ctx.vg);
    nvgScissor(ctx.vg, 0, 0, container_width, container_height);
    nvgTranslate(ctx.vg, -state->scroll_x, -state->scroll_y);
  
    auto child_ctx = ctx;
    child_ctx.x -= state->scroll_x;
    child_ctx.y -= state->scroll_y;
    child_ctx.width = container_width > inner_width ? container_width : inner_width;
    child_ctx.height = container_height > inner_height ? container_height : inner_height;
    child_ctx.clip.x1 = ctx.x;
    child_ctx.clip.y1 = ctx.y;
    child_ctx.clip.x2 = ctx.x + container_width;
    child_ctx.clip.y2 = ctx.y + container_height;
    update_inner(child_ctx);
  
    nvgRestore(ctx.vg);
  
    // Draw scroll bars
    if (show_h_bar) {
        NVGcolor color = nvgRGBA(0, 0, 0, 76);
        if (state->is_dragging_horizontal_bar) {
            color = nvgRGBA(42, 159, 214, 205); // #2a9fd6
        } else if (mouse_over(ctx, h_bar_x, h_bar_y, h_bar_w, h_bar_h)) {
            color = nvgRGBA(100, 100, 100, 205);
        }
        nvgBeginPath(ctx.vg);
        nvgFillColor(ctx.vg, color);
        nvgRect(ctx.vg, h_bar_x, h_bar_y, h_bar_w, h_bar_h);
        nvgFill(ctx.vg);
    }
    if (show_v_bar) {
        NVGcolor color = nvgRGBA(0, 0, 0, 76);
        if (state->is_dragging_vertical_bar) {
            color = nvgRGBA(42, 159, 214, 205); // #2a9fd6
        } else if (mouse_over(ctx, v_bar_x, v_bar_y, v_bar_w, v_bar_h)) {
            color = nvgRGBA(100, 100, 100, 205);
        }
        nvgBeginPath(ctx.vg);
        nvgFillColor(ctx.vg, color);
        nvgRect(ctx.vg, v_bar_x, v_bar_y, v_bar_w, v_bar_h);
        nvgFill(ctx.vg);
    }
}

void scroll_into_view(ScrollAreaState* state, Context ctx, int x, int y, int width, int height) {

    state->scroll_into_view.x = x;
    state->scroll_into_view.y = y;
    state->scroll_into_view.width = width;
    state->scroll_into_view.height = height;
    *ctx.must_repaint = true;

}

}
