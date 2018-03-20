//
//  PasswordTextBox.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 20/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "PasswordTextBox.hpp"
#include <ddui/keyboard>
#include <ddui/util/caret_flicker>
#include <cstdlib>

namespace PasswordTextBox {

PasswordTextBoxState::PasswordTextBoxState() {
    current_version_count = -1;
}

void update(PasswordTextBoxState* state, Context ctx) {

    keyboard::register_focus_group(ctx, state);
    
    // Process key input
    if (keyboard::has_key_event(ctx, state)) {
        if (!(ctx.key->key == keyboard::KEY_ENTER) &&
            !(ctx.key->key == keyboard::KEY_C && ctx.key->mods == keyboard::MOD_SUPER)) {
            TextEdit::apply_keyboard_input(state->model, ctx.key);
            TextEdit::remove_line_breaks(state->model);
        }
    }
    
    // When tabbing in to focus, select the entire content
    if (keyboard::did_focus(ctx, state) && !state->is_mouse_dragging) {
        auto& selection = state->model->selection;
        if (selection.a_line  == selection.b_line &&
            selection.a_index == selection.b_index) {
            selection.a_line = 0;
            selection.a_index = 0;
            selection.b_line = state->model->lines.size() - 1;
            selection.b_index = state->model->lines.back().characters.size();
            state->model->version_count++;
        }
    }
    
    // Reset the flickering caret to ON whenever the model has changed
    if (keyboard::has_focus(ctx, state) && state->current_version_count != state->model->version_count) {
        caret_flicker::reset_phase();
    }

    // Refresh the model measurements
    if (state->model->version_count != state->current_version_count) {
        state->measurements = TextEdit::measure(ctx, state->model, std::function<void(Context,int,int*,int*)>());
        state->current_version_count = state->model->version_count;
    }
    
    // Update the text box height
    state->height = state->measurements.height + 2 * state->margin;

    // Focus the box on a mouse click
    if (!keyboard::has_focus(ctx, state) && mouse_hit(ctx, 0, 0, ctx.width, state->height)) {
        keyboard::focus(ctx, state);
        caret_flicker::reset_phase();
    }
    
    // Calculate the dot size
    int dot_bounding_height = state->measurements.height;
    int dot_bounding_width  = dot_bounding_height * 0.6;
    int dot_diameter        = dot_bounding_width * 0.8;
    int dot_margin          = (dot_bounding_width - dot_diameter) / 2;
    int num_dots            = state->model->lines.front().characters.size();
    int total_width         = num_dots * dot_bounding_width;
    
    // Update selection by mouse dragging (it's initiated at the end of this function)
    if (state->is_mouse_dragging && !ctx.mouse->pressed) {
        state->is_mouse_dragging = false;
    }
    if (state->is_mouse_dragging) {
        *ctx.cursor = CURSOR_IBEAM;
        int x = ctx.mouse->x - ctx.x - state->margin + state->scroll_x;
        state->model->selection.b_line = 0;
        if (x < dot_bounding_width / 2) {
            state->model->selection.b_index = 0;
        } else if (x > num_dots * dot_bounding_width - dot_bounding_width / 2) {
            state->model->selection.b_index = num_dots;
        } else {
            state->model->selection.b_index = (int)((float)(x + dot_bounding_width / 2) / dot_bounding_width);
        }
    }

    // Background
    nvgBeginPath(ctx.vg);
    nvgFillColor(ctx.vg, keyboard::has_focus(ctx, state) ? state->bg_color_focused : state->bg_color);
    nvgRoundedRect(ctx.vg, 0, 0, ctx.width, state->height, state->border_radius);
    nvgFill(ctx.vg);
    
    // Border
    nvgBeginPath(ctx.vg);
    nvgStrokeColor(ctx.vg, keyboard::has_focus(ctx, state) ? state->border_color_focused : state->border_color);
    nvgStrokeWidth(ctx.vg, state->border_width);
    nvgRoundedRect(ctx.vg, 0, 0, ctx.width, state->height, state->border_radius);
    nvgStroke(ctx.vg);
    
    // Update scrolling
    int caret_x = state->model->selection.b_index * dot_bounding_width;
    while (caret_x < state->scroll_x) {
        state->scroll_x -= 50;
    }
    while (caret_x > state->scroll_x + ctx.width - 2 * state->margin) {
        state->scroll_x += 50;
    }
    if (state->scroll_x > total_width - ctx.width + 2 * state->margin) {
        state->scroll_x = total_width - ctx.width + 2 * state->margin;
    }
    if (state->scroll_x < 0) {
        state->scroll_x = 0;
    }
    
    // Prepare clip-region
    nvgSave(ctx.vg);
    nvgScissor(ctx.vg, 0, 0, ctx.width, state->height);
    nvgTranslate(ctx.vg, -state->scroll_x, 0);
    
    auto inner_width = total_width + 2 * state->margin;
    
    auto child_ctx = ctx;
    child_ctx.x -= state->scroll_x;
    child_ctx.width = inner_width > ctx.width ? inner_width : ctx.width;
    child_ctx.clip.x1 = ctx.x;
    child_ctx.clip.y1 = ctx.y;
    child_ctx.clip.x2 = ctx.x + ctx.width;
    child_ctx.clip.y2 = ctx.y + state->height;
    
    // Text (when selection in foreground)
    if (state->selection_in_foreground) {
        float x = state->margin + dot_bounding_width / 2;
        float y = state->margin + dot_bounding_height / 2;
        nvgFillColor(ctx.vg, state->model->lines.front().style.text_color);
        for (int i = 0; i < num_dots; ++i) {
            nvgBeginPath(ctx.vg);
            nvgCircle(ctx.vg, x, y, dot_diameter / 2);
            nvgFill(ctx.vg);
            x += dot_bounding_width;
        }
    }
    
    // Selection
    if (keyboard::has_focus(ctx, state)) {
        auto& sel = state->model->selection;
        
        if (sel.a_index == sel.b_index) {
            // Cursor
        
            auto cursor_color = (state->is_mouse_dragging || caret_flicker::get_phase()) ? state->cursor_color : nvgRGBA(0, 0, 0, 0);
            nvgBeginPath(ctx.vg);
            nvgStrokeColor(ctx.vg, cursor_color);
            nvgStrokeWidth(ctx.vg, 2.0);
            nvgMoveTo(ctx.vg, state->margin + sel.a_index * dot_bounding_width, state->margin);
            nvgLineTo(ctx.vg, state->margin + sel.a_index * dot_bounding_width, state->margin + dot_bounding_height);
            nvgStroke(ctx.vg);
        } else {
            // Selection
            
            auto from = sel.a_index < sel.b_index ? sel.a_index : sel.b_index;
            auto to   = sel.a_index > sel.b_index ? sel.a_index : sel.b_index;
            
            nvgBeginPath(ctx.vg);
            nvgFillColor(ctx.vg, state->selection_color);
            nvgRect(ctx.vg, state->margin + from * dot_bounding_width, state->margin,
                            (to - from) * dot_bounding_width, dot_bounding_height);
            nvgFill(ctx.vg);
        }
    }

    // Text (when selection in background)
    if (!state->selection_in_foreground) {
        float x = state->margin;
        float y = state->margin + dot_bounding_height / 2;
        nvgFillColor(ctx.vg, state->model->lines.front().style.text_color);
        for (int i = 0; i < num_dots; ++i) {
            nvgBeginPath(ctx.vg);
            nvgRect(ctx.vg, x + dot_margin, y - dot_diameter / 2, dot_diameter, dot_diameter);
            nvgFill(ctx.vg);
            x += dot_bounding_width;
        }
    }
    
    // Dispose of clip region
    nvgRestore(ctx.vg);

    // Initiate selection by mouse dragging
    if (mouse_hit(ctx, 0, 0, ctx.width, state->height)) {
        *ctx.cursor = CURSOR_IBEAM;
        ctx.mouse->accepted = true;
        state->is_mouse_dragging = true;
        
        int x = ctx.mouse->x - ctx.x - state->margin + state->scroll_x;
        
        state->model->selection.a_line = 0;
        if (x < dot_bounding_width / 2) {
            state->model->selection.a_index = 0;
        } else if (x > num_dots * dot_bounding_width - dot_bounding_width / 2) {
            state->model->selection.a_index = num_dots;
        } else {
            state->model->selection.a_index = (int)((float)(x + dot_bounding_width / 2) / dot_bounding_width);
        }
        state->model->selection.b_line = state->model->selection.a_line;
        state->model->selection.b_index = state->model->selection.a_index;
        caret_flicker::reset_phase();
    }
    if (mouse_over(ctx, 0, 0, ctx.width, state->height)) {
        *ctx.cursor = CURSOR_IBEAM;
    }

}

}
