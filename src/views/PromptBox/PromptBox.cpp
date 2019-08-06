//
//  PromptBox.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 06/08/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "PromptBox.hpp"
#include <ddui/views/Overlay>

namespace PromptBox {

using namespace ddui;

static void update_content(PromptBoxState* state);
static bool draw_button(void* identifier, float y, float* x, const char* text, bool disabled);

PromptBoxState::PromptBoxState() {
    opened = false;
    overlay_box.max_width = 350;
    overlay_box.max_height = 180;
    action = -1;

    text_box_model.regular_font = "regular";
    
    TextEdit::set_style(&text_box_model, false, 18.0, rgb(0xffffff));
}

void open(PromptBoxState* state) {
    state->action = -1;
    OverlayBox::open(&state->overlay_box);
}

void close(PromptBoxState* state) {
    OverlayBox::close(&state->overlay_box);
}

void update(PromptBoxState* state) {
    OverlayBox::update(&state->overlay_box, [state]() {
        update_content(state);
    });
}

constexpr auto PADDING = 10;
constexpr auto TITLE_SIZE = 20;
constexpr auto MESSAGE_SIZE = 18;
static auto TITLE_TEXT_COLOR = rgb(0xcccccc);
static auto MESSAGE_TEXT_COLOR = rgb(0xffffff);

void update_content(PromptBoxState* state) {

    // Detect opening and closing
    auto ANIMATION_IN_ID = (void*)(&state->overlay_box + 1);
    auto ANIMATION_OUT_ID = (void*)(&state->overlay_box + 2);
    if (!state->opened &&
        !animation::is_animating(ANIMATION_IN_ID) &&
        !animation::is_animating(ANIMATION_OUT_ID)) {
        state->opened = true;
        focus(&state->text_box_state);
    }
    if (state->opened &&
        animation::is_animating(ANIMATION_OUT_ID)) {
        state->opened = false;
    }

    auto y = 0;
    
    // Title
    font_face("regular");
    font_size(TITLE_SIZE);
    fill_color(TITLE_TEXT_COLOR);
    text_align(align::TOP | align::CENTER);
    text(view.width / 2, y + PADDING, state->title.c_str(), NULL);
    text_align(align::LEFT);
    y += TITLE_SIZE + 2 * PADDING;

    // Message
    font_face("regular");
    font_size(MESSAGE_SIZE);
    fill_color(MESSAGE_TEXT_COLOR);
    text_align(align::TOP | align::LEFT);
    text_box(PADDING, y, view.width - 2 * PADDING, state->message.c_str(), NULL);
    text_align(align::LEFT);

    // Submit Key (ENTER)
    if (has_key_event(&state->text_box_state) &&
        key_state.key == keyboard::KEY_ENTER) {
        if (key_state.action == keyboard::ACTION_PRESS) {
            close(state);
            state->action = 0;
        }
        consume_key_event();
    }

    // Plain Text Box
    sub_view(PADDING, y + MESSAGE_SIZE * 3, view.width - 2 * PADDING, view.height);
    {
        auto styles = *PlainTextBox::get_global_styles();
        styles.bg_color = rgb(0x282828);
        styles.bg_color_focused = rgb(0x282828);
        styles.border_color = rgb(0x606060);
        styles.border_color_focused = rgb(0xaaaaaa);
        styles.cursor_color = rgb(0xffffff);
        styles.selection_color = rgba(0xffffff, 0.5);

        PlainTextBox(&state->text_box_state, &state->text_box_model)
            .set_styles(&styles)
            .update();
    }
    restore();

    // Register Buttons as Focus Groups
    for (auto& button : state->button_set) {
        register_focus_group(&button);
    }

    // Draw Buttons
    auto x = view.width - PADDING;
    y = view.height - PADDING;

    for (int i = state->button_set.size() - 1; i >= 0; --i) {
        void* id = &state->button_set[i];
        if (draw_button(id, y, &x, state->button_set[i].c_str(), false)) {
            close(state);
            state->action = i;
        }
    }
    
    if (mouse_hit(0, 0, view.width, view.height)) {
        mouse_hit_accept();
    }

}

constexpr auto BUTTON_TEXT_SIZE = 20;
constexpr auto BUTTON_H_PADDING = 16;
constexpr auto BUTTON_V_PADDING = 4;
constexpr auto BUTTON_SPACING = 4;
constexpr auto BUTTON_BORDER_RADIUS = 4;

bool draw_button(void* identifier, float y, float* x, const char* text, bool disabled) {
    
    font_face("regular");
    font_size(BUTTON_TEXT_SIZE);
    
    float ascender, descender, line_height;
    text_metrics(&ascender, &descender, &line_height);
    auto button_height = 2 * BUTTON_V_PADDING + line_height;
    y -= button_height;
    
    float bounds[4];
    text_bounds(0, 0, text, NULL, bounds);
    
    auto text_width = bounds[2] - bounds[0];
    auto button_width = 2 * BUTTON_H_PADDING + text_width;
    
    *x -= button_width;
    
    auto hovering = mouse_over(*x, y, button_width, button_height);
    if (hovering) {
        set_cursor(CURSOR_POINTING_HAND);
    }
    hovering |= has_focus(identifier);
    
    // Background
    begin_path();
    rounded_rect(*x, y, button_width, button_height, BUTTON_BORDER_RADIUS);
    fill_color(disabled ? rgb(0x282828) : hovering ? rgb(0x666666) : rgb(0x343434));
    fill();
    
    // Text
    fill_color(disabled ? rgb(0x666666) : rgb(0xffffff));
    ddui::text(*x + BUTTON_H_PADDING, y + BUTTON_V_PADDING + ascender, text, NULL);
    
    bool clicked = mouse_hit(*x, y, button_width, button_height);
    if (clicked) {
        mouse_hit_accept();
    }

    *x -= BUTTON_SPACING;

    // Enter to Submit
    if (has_key_event(identifier) &&
        key_state.action == keyboard::ACTION_PRESS &&
        key_state.key == keyboard::KEY_ENTER) {
        consume_key_event();
        clicked = true;
    }

    return clicked;
}

}
