//
//  OverlayBox.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 06/08/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "OverlayBox.hpp"
#include <ddui/app>
#include <ddui/views/Overlay>
#include <ddui/animation>

namespace OverlayBox {

static void update_overlay(OverlayBoxState* state, Context ctx);

constexpr auto DURATION_FADE_IN = 0.35;
constexpr auto DURATION_FADE_OUT = 0.35;

static auto BORDER_COLOR = nvgRGB(0x88, 0x88, 0x88);

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

void update(OverlayBoxState* state, std::function<void(Context)> update_content) {
    auto OVERLAY_ID = (void*)state;
    state->update_content = std::move(update_content);
    Overlay::handle_overlay(OVERLAY_ID, [state](Context ctx) {
        update_overlay(state, ctx);
    });
}

void update_overlay(OverlayBoxState* state, Context ctx) {

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

    nvgBeginPath(ctx.vg);
    nvgRect(ctx.vg, 0, 0, ctx.width, ctx.height);
    nvgFillColor(ctx.vg, nvgRGBAf(0.4, 0.4, 0.4, 0.4 * (is_closing ? 1 - completion : completion)));
    nvgFill(ctx.vg);

    constexpr auto MARGIN = 5;

    auto overlay_width = state->max_width;
    if (overlay_width > ctx.width - 2 * MARGIN) {
        overlay_width = ctx.width - 2 * MARGIN;
    }

    auto overlay_height = state->max_height;
    if (overlay_height > ctx.height - 2 * MARGIN) {
        overlay_height = ctx.height - 2 * MARGIN;
    }

    double x  = (ctx.width  - overlay_width)  / 2;
    double y2 = (ctx.height - overlay_height) / 2;
    double y1 = y2 - 200;

    double ratio = is_closing ? animation::ease_in(1 - completion) : animation::ease_out(completion);
    double y = y1 + (y2 - y1) * ratio;

    auto child_ctx = child_context(ctx, x, y, overlay_width, overlay_height);
    {
        nvgGlobalAlpha(ctx.vg, ratio);

        // Background
        nvgBeginPath(ctx.vg);
        nvgRoundedRect(ctx.vg, 0, 0, overlay_width, overlay_height, 4);
        nvgFillColor(ctx.vg, nvgRGB(0x11, 0x11, 0x11));
        nvgFill(ctx.vg);
        
        // Border outline
        nvgStrokeColor(ctx.vg, BORDER_COLOR);
        nvgStrokeWidth(ctx.vg, 1.0);
        nvgStroke(ctx.vg);

        // Content
        state->update_content(child_ctx);
        state->update_content = std::function<void(Context)>();

    }
    nvgRestore(ctx.vg);

    if (mouse_hit(ctx, 0, 0, ctx.width, ctx.height)) {
        ctx.mouse->accepted = true;
        if (!animation::is_animating(ANIMATION_OUT_ID)) {
            close(state);
        }
    }
}

}
