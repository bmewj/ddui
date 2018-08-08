//
//  MessageBox.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 06/08/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "MessageBox.hpp"
#include <ddui/keyboard>

namespace MessageBox {

static void update_content(MessageBoxState* state, Context ctx);
static bool draw_button(Context ctx, void* identifier, int y, int* x, const char* text, bool disabled);

MessageBoxState::MessageBoxState() {
    overlay_box.max_width = 350;
    overlay_box.max_height = 150;
    action = -1;
}

void open(MessageBoxState* state) {
    state->action = -1;
    OverlayBox::open(&state->overlay_box);
}

void close(MessageBoxState* state) {
    OverlayBox::close(&state->overlay_box);
}

void update(MessageBoxState* state) {
    OverlayBox::update(&state->overlay_box, [state](Context ctx) {
        update_content(state, ctx);
    });
}

constexpr auto PADDING = 10;
constexpr auto TITLE_SIZE = 20;
constexpr auto MESSAGE_SIZE = 18;
static auto TITLE_TEXT_COLOR = nvgRGB(0xcc, 0xcc, 0xcc);
static auto MESSAGE_TEXT_COLOR = nvgRGB(0xff, 0xff, 0xff);

void update_content(MessageBoxState* state, Context ctx) {

    auto y = 0;
    
    // Title
    nvgFontFace(ctx.vg, "regular");
    nvgFontSize(ctx.vg, TITLE_SIZE);
    nvgFillColor(ctx.vg, TITLE_TEXT_COLOR);
    nvgTextAlign(ctx.vg, NVG_ALIGN_TOP | NVG_ALIGN_CENTER);
    nvgText(ctx.vg, ctx.width / 2, y + PADDING, state->title.c_str(), NULL);
    nvgTextAlign(ctx.vg, NVG_ALIGN_LEFT);
    y += TITLE_SIZE + 2 * PADDING;

    // Message
    nvgFontFace(ctx.vg, "regular");
    nvgFontSize(ctx.vg, MESSAGE_SIZE);
    nvgFillColor(ctx.vg, MESSAGE_TEXT_COLOR);
    nvgTextAlign(ctx.vg, NVG_ALIGN_TOP | NVG_ALIGN_LEFT);
    nvgTextBox(ctx.vg, PADDING, y, ctx.width - 2 * PADDING, state->message.c_str(), NULL);
    nvgTextAlign(ctx.vg, NVG_ALIGN_LEFT);

    // Register Buttons as Focus Groups
    for (auto& button : state->button_set) {
        keyboard::register_focus_group(ctx, &button);
    }

    // Draw Buttons
    auto x = ctx.width - PADDING;
    y = ctx.height - PADDING;

    for (int i = state->button_set.size() - 1; i >= 0; --i) {
        void* id = &state->button_set[i];
        if (draw_button(ctx, id, y, &x, state->button_set[i].c_str(), false)) {
            close(state);
            state->action = i;
        }
    }
    
    if (mouse_hit(ctx, 0, 0, ctx.width, ctx.height)) {
        ctx.mouse->accepted = true;
    }

}

constexpr auto BUTTON_TEXT_SIZE = 20;
constexpr auto BUTTON_H_PADDING = 16;
constexpr auto BUTTON_V_PADDING = 4;
constexpr auto BUTTON_SPACING = 4;
constexpr auto BUTTON_BORDER_RADIUS = 4;

bool draw_button(Context ctx, void* identifier, int y, int* x, const char* text, bool disabled) {

    nvgFontFace(ctx.vg, "regular");
    nvgFontSize(ctx.vg, BUTTON_TEXT_SIZE);
    
    float ascender, descender, line_height;
    nvgTextMetrics(ctx.vg, &ascender, &descender, &line_height);
    auto button_height = 2 * BUTTON_V_PADDING + line_height;
    y -= button_height;
    
    float bounds[4];
    nvgTextBounds(ctx.vg, 0, 0, text, NULL, bounds);
    
    auto text_width = bounds[2] - bounds[0];
    auto button_width = 2 * BUTTON_H_PADDING + text_width;
    
    *x -= button_width;
    
    auto hovering = mouse_over(ctx, *x, y, button_width, button_height);
    if (hovering) {
        *ctx.cursor = CURSOR_POINTING_HAND;
    }
    hovering |= keyboard::has_focus(ctx, identifier);

    // Background
    nvgBeginPath(ctx.vg);
    nvgRoundedRect(ctx.vg, *x, y, button_width, button_height, BUTTON_BORDER_RADIUS);
    nvgFillColor(ctx.vg, disabled ? nvgRGB(0x28, 0x28, 0x28) : hovering ? nvgRGB(0x66, 0x66, 0x66) : nvgRGB(0x34, 0x34, 0x34));
    nvgFill(ctx.vg);
    
    // Text
    nvgFillColor(ctx.vg, disabled ? nvgRGB(0x66, 0x66, 0x66) : nvgRGB(0xff, 0xff, 0xff));
    nvgText(ctx.vg, *x + BUTTON_H_PADDING, y + BUTTON_V_PADDING + ascender, text, NULL);
    
    bool clicked = mouse_hit(ctx, *x, y, button_width, button_height);
    if (clicked) {
        ctx.mouse->accepted = true;
    }

    *x -= BUTTON_SPACING;

    // Enter to Submit
    if (keyboard::has_key_event(ctx, identifier) &&
        ctx.key->action == keyboard::ACTION_PRESS &&
        ctx.key->key == keyboard::KEY_ENTER) {
        keyboard::consume_key_event(ctx);
        clicked = true;
    }

    return clicked;
}

}
