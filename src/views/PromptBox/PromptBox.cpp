//
//  PromptBox.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 06/08/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "PromptBox.hpp"
#include <ddui/keyboard>
#include <ddui/animation>
#include <ddui/views/Overlay>

namespace PromptBox {

static void update_content(PromptBoxState* state, Context ctx);
static bool draw_button(Context ctx, int y, int* x, const char* text, bool disabled);

PromptBoxState::PromptBoxState() {
    opened = false;
    overlay_box.max_width = 350;
    overlay_box.max_height = 180;
    action = -1;

    text_box_model.regular_font = "regular";
    
    text_box.model = &text_box_model;
    text_box.multiline = false;
    
    TextEdit::set_style(&text_box_model, false, 18.0, nvgRGB(0xff, 0xff, 0xff));
    
    text_box.bg_color = nvgRGB(0x28, 0x28, 0x28);
    text_box.bg_color_focused = nvgRGB(0x28, 0x28, 0x28);
    text_box.border_color = nvgRGB(0x60, 0x60, 0x60);
    text_box.border_color_focused = nvgRGB(0xaa, 0xaa, 0xaa);
    text_box.cursor_color = nvgRGB(0xff, 0xff, 0xff);
    text_box.selection_color = nvgRGBA(0xff, 0xff, 0xff, 0x80);
}

void open(PromptBoxState* state) {
    state->action = -1;
    OverlayBox::open(&state->overlay_box);
}

void close(PromptBoxState* state) {
    OverlayBox::close(&state->overlay_box);
}

void update(PromptBoxState* state) {
    OverlayBox::update(&state->overlay_box, [state](Context ctx) {
        update_content(state, ctx);
    });
}

constexpr auto PADDING = 10;
constexpr auto TITLE_SIZE = 20;
constexpr auto MESSAGE_SIZE = 18;
static auto TITLE_TEXT_COLOR = nvgRGB(0xcc, 0xcc, 0xcc);
static auto MESSAGE_TEXT_COLOR = nvgRGB(0xff, 0xff, 0xff);

void update_content(PromptBoxState* state, Context ctx) {

    // Detect opening and closing
    auto ANIMATION_IN_ID = (void*)(&state->overlay_box + 1);
    auto ANIMATION_OUT_ID = (void*)(&state->overlay_box + 2);
    if (!state->opened &&
        !animation::is_animating(ANIMATION_IN_ID) &&
        !animation::is_animating(ANIMATION_OUT_ID)) {
        state->opened = true;
        keyboard::focus(ctx, &state->text_box);
    }
    if (state->opened &&
        animation::is_animating(ANIMATION_OUT_ID)) {
        state->opened = false;
    }

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

    // Submit Key (ENTER)
    if (keyboard::has_key_event(ctx, &state->text_box) &&
        ctx.key->key == keyboard::KEY_ENTER) {
        if (ctx.key->action == keyboard::ACTION_PRESS) {
            close(state);
            state->action = 0;
        }
        keyboard::consume_key_event(ctx);
    }

    // Plain Text Box
    auto child_ctx = child_context(ctx, PADDING, y + MESSAGE_SIZE * 3, ctx.width - 2 * PADDING, ctx.height);
    PlainTextBox::update(&state->text_box, child_ctx);
    nvgRestore(child_ctx.vg);

    // Buttons
    auto x = ctx.width - PADDING;
    y = ctx.height - PADDING;

    for (int i = state->button_set.size() - 1; i >= 0; --i) {
        if (draw_button(ctx, y, &x, state->button_set[i].c_str(), false)) {
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

bool draw_button(Context ctx, int y, int* x, const char* text, bool disabled) {
    
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

    return clicked;
}

}
