//
//  PasswordTextBox.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 20/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "PasswordTextBox.hpp"
#include <ddui/util/caret_flicker>
#include <cstdlib>

namespace PasswordTextBox {

using namespace ddui;

PasswordTextBoxState::PasswordTextBoxState() {
    current_version_count = -1;
    scroll_x = 0;
}

void update(PasswordTextBoxState* state) {

    register_focus_group(state);
    
    // Process key input
    if (has_key_event(state)) {
        if (!(key_state.key == keyboard::KEY_ENTER) &&
            !(key_state.key == keyboard::KEY_KP_ENTER) &&
            !(key_state.key == keyboard::KEY_C && key_state.mods == keyboard::MOD_COMMAND)) {
            TextEdit::apply_keyboard_input(state->model, &key_state);
            TextEdit::remove_line_breaks(state->model);
        }
    }
    
    // When tabbing in to focus, select the entire content
    if (did_focus(state) && !state->is_mouse_dragging) {
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
    if (has_focus(state) && state->current_version_count != state->model->version_count) {
        caret_flicker::reset_phase();
    }

    // Refresh the model measurements
    if (state->model->version_count != state->current_version_count) {
        state->measurements = TextEdit::measure(state->model, std::function<void(float,float*,float*)>());
        state->current_version_count = state->model->version_count;
    }
    
    // Update the text box height
    state->height = state->measurements.height + 2 * state->margin;

    // Focus the box on a mouse click
    if (!has_focus(state) && mouse_hit(0, 0, view.width, state->height)) {
        focus(state);
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
    if (state->is_mouse_dragging && !mouse_state.pressed) {
        state->is_mouse_dragging = false;
    }
    if (state->is_mouse_dragging) {
        set_cursor(CURSOR_IBEAM);

        float mx, my, x;
        mouse_position(&mx, &my);
        x = mx - state->margin + state->scroll_x;

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
    begin_path();
    fill_color(has_focus(state) ? state->bg_color_focused : state->bg_color);
    rounded_rect(0, 0, view.width, state->height, state->border_radius);
    fill();
    
    // Border
    begin_path();
    stroke_color(has_focus(state) ? state->border_color_focused : state->border_color);
    stroke_width(state->border_width);
    rounded_rect(0, 0, view.width, state->height, state->border_radius);
    stroke();
    
    // Update scrolling
    int caret_x = state->model->selection.b_index * dot_bounding_width;
    while (caret_x < state->scroll_x) {
        state->scroll_x -= 50;
    }
    while (caret_x > state->scroll_x + view.width - 2 * state->margin) {
        state->scroll_x += 50;
    }
    if (state->scroll_x > total_width - view.width + 2 * state->margin) {
        state->scroll_x = total_width - view.width + 2 * state->margin;
    }
    if (state->scroll_x < 0) {
        state->scroll_x = 0;
    }
    
    // Prepare clip-region
    auto inner_width = total_width + 2 * state->margin;
    save();
    clip(0, 0, view.width, state->height);
    sub_view(-state->scroll_x, 0, inner_width > view.width ? inner_width : view.width, state->height);
    
    // Text (when selection in foreground)
    if (state->selection_in_foreground) {
        float x = state->margin + dot_bounding_width / 2;
        float y = state->margin + dot_bounding_height / 2;
        fill_color(state->model->lines.front().style.text_color);
        for (int i = 0; i < num_dots; ++i) {
            begin_path();
            circle(x, y, dot_diameter / 2);
            fill();
            x += dot_bounding_width;
        }
    }
    
    // Selection
    if (has_focus(state)) {
        auto& sel = state->model->selection;
        
        if (sel.a_index == sel.b_index) {
            // Cursor
        
            auto cursor_color = (state->is_mouse_dragging || caret_flicker::get_phase()) ? state->cursor_color : rgba(0x000000, 0.0);
            begin_path();
            stroke_color(cursor_color);
            stroke_width(2.0);
            move_to(state->margin + sel.a_index * dot_bounding_width, state->margin);
            line_to(state->margin + sel.a_index * dot_bounding_width, state->margin + dot_bounding_height);
            stroke();
        } else {
            // Selection
            
            auto from = sel.a_index < sel.b_index ? sel.a_index : sel.b_index;
            auto to   = sel.a_index > sel.b_index ? sel.a_index : sel.b_index;
            
            begin_path();
            fill_color(state->selection_color);
            rect(state->margin + from * dot_bounding_width, state->margin,
                            (to - from) * dot_bounding_width, dot_bounding_height);
            fill();
        }
    }

    // Text (when selection in background)
    if (!state->selection_in_foreground) {
        float x = state->margin;
        float y = state->margin + dot_bounding_height / 2;
        fill_color(state->model->lines.front().style.text_color);
        for (int i = 0; i < num_dots; ++i) {
            begin_path();
            rect(x + dot_margin, y - dot_diameter / 2, dot_diameter, dot_diameter);
            fill();
            x += dot_bounding_width;
        }
    }
    
    // Dispose of clip region
    restore();
    restore();

    // Initiate selection by mouse dragging
    if (mouse_hit(0, 0, view.width, state->height)) {
        set_cursor(CURSOR_IBEAM);
        mouse_hit_accept();
        state->is_mouse_dragging = true;

        float mx, my, x;
        mouse_position(&mx, &my);
        x = mx - state->margin + state->scroll_x;
        
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
    if (mouse_over(0, 0, view.width, state->height)) {
        set_cursor(CURSOR_IBEAM);
    }

}

}
