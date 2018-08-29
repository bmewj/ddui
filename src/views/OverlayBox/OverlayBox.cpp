//
//  OverlayBox.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 06/08/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "OverlayBox.hpp"
#include "style.hpp"
#include <ddui/core>
#include <ddui/views/Overlay>

namespace OverlayBox {

using namespace ddui;

static void update_overlay(OverlayBoxState* state);

constexpr auto DURATION_FADE_IN = 0.35;
constexpr auto DURATION_FADE_OUT = 0.35;

OverlayBoxState::OverlayBoxState() {
    max_width = 400;
    max_height = 500;
}

void open(OverlayBoxState* state) {
    auto OVERLAY_ID = (void*)state;
    auto ANIMATION_IN_ID = (void*)(state + 1);
    animation::start(ANIMATION_IN_ID);
    Overlay::open(OVERLAY_ID);
}

void close(OverlayBoxState* state) {
    auto ANIMATION_OUT_ID = (void*)(state + 2);
    animation::start(ANIMATION_OUT_ID);
}

void update(OverlayBoxState* state, std::function<void()> update_content) {
    auto OVERLAY_ID = (void*)state;
    state->update_content = std::move(update_content);
    Overlay::handle_overlay(OVERLAY_ID, [state]() {
        update_overlay(state);
    });
}

void update_overlay(OverlayBoxState* state) {

    auto OVERLAY_ID = (void*)state;
    auto ANIMATION_IN_ID = (void*)(state + 1);
    auto ANIMATION_OUT_ID = (void*)(state + 2);

    auto completion = 1.0;

    auto is_opening = animation::is_animating(ANIMATION_IN_ID);
    if (is_opening) {
        completion = animation::get_time_elapsed(ANIMATION_IN_ID) / DURATION_FADE_IN;
        if (completion >= 1.0) {
            animation::stop(ANIMATION_IN_ID);
            completion = 1.0;
        }
    }

    auto is_closing = animation::is_animating(ANIMATION_OUT_ID);
    if (is_closing) {
        completion = animation::get_time_elapsed(ANIMATION_OUT_ID) / DURATION_FADE_OUT;
        if (completion >= 1.0) {
            animation::stop(ANIMATION_OUT_ID);
            Overlay::close(OVERLAY_ID);
            return;
        }
    }

    auto screen_fill_color = style::SCREEN_FILL_COLOR;
    screen_fill_color.a *= (is_closing ? 1 - completion : completion);

    begin_path();
    rect(0, 0, view.width, view.height);
    fill_color(screen_fill_color);
    fill();

    auto overlay_width = state->max_width;
    if (overlay_width > view.width - 2 * style::BOX_MARGIN) {
        overlay_width = view.width - 2 * style::BOX_MARGIN;
    }

    auto overlay_height = state->max_height;
    if (overlay_height > view.height - 2 * style::BOX_MARGIN) {
        overlay_height = view.height - 2 * style::BOX_MARGIN;
    }

    double x  = (view.width  - overlay_width)  / 2;
    double y2 = (view.height - overlay_height) / 2;
    double y1 = y2 - 200;

    double ratio = is_closing ? animation::ease_in(1 - completion) : animation::ease_out(completion);
    double y = y1 + (y2 - y1) * ratio;

    sub_view(x, y, overlay_width, overlay_height);
    {
        global_alpha(ratio);

        // Background
        begin_path();
        rounded_rect(0, 0, overlay_width, overlay_height, style::BORDER_RADIUS);
        fill_color(style::BACKGROUND_COLOR);
        fill();

        // Border outline
        stroke_color(style::BORDER_COLOR);
        stroke_width(style::BORDER_WIDTH);
        stroke();

        // Content
        auto old_max_height = state->max_height;
        state->update_content();
        state->update_content = std::function<void()>();
        if (old_max_height != state->max_height) {
            repaint();
        }

    }
    restore();

    if (mouse_hit(0, 0, view.width, view.height)) {
        mouse_hit_accept();
        if (!animation::is_animating(ANIMATION_OUT_ID)) {
            close(state);
        }
    }
}

}
