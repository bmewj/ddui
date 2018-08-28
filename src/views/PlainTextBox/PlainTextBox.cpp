//
//  PlainTextBox.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 18/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "PlainTextBox.hpp"
#include <ddui/util/caret_flicker>
#include <cstdlib>

namespace PlainTextBox {

using namespace ddui;

PlainTextBoxState::PlainTextBoxState() {
    current_version_count = -1;
}

void update(PlainTextBoxState* state) {

    register_focus_group(state);
    
    // Process key input
    if (has_key_event(state)) {
        if (state->multiline || key_state.key != keyboard::KEY_ENTER) {
            TextEdit::apply_keyboard_input(state->model, &key_state);
        }
        if (!state->multiline) {
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
    
    // Update selection by mouse dragging (it's initiated at the end of this function)
    if (state->is_mouse_dragging && !mouse_state.pressed) {
        state->is_mouse_dragging = false;
    }
    if (state->is_mouse_dragging) {
        set_cursor(CURSOR_IBEAM);

        float mx, my, x, y;
        mouse_position(&mx, &my);
        x = mx - state->margin + state->scroll_x;
        y = my - state->margin;
        
        TextEdit::locate_selection_point(&state->measurements, x, y,
                                         &state->model->selection.b_line,
                                         &state->model->selection.b_index);
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
    int caret_x = (state->model->selection.b_index == 0 ? 0 :
                   state->measurements.lines[state->model->selection.b_line]
                                 .characters[state->model->selection.b_index - 1].max_x);
    while (caret_x < state->scroll_x) {
        state->scroll_x -= 50;
    }
    while (caret_x > state->scroll_x + view.width - 2 * state->margin) {
        state->scroll_x += 50;
    }
    if (state->scroll_x > state->measurements.width - view.width + 2 * state->margin) {
        state->scroll_x = state->measurements.width - view.width + 2 * state->margin;
    }
    if (state->scroll_x < 0) {
        state->scroll_x = 0;
    }
    
    // Prepare clip-region
    auto inner_width = state->measurements.width + 2 * state->margin;
    save();
    clip(0, 0, view.width, state->height);
    sub_view(-state->scroll_x, 0, inner_width > view.width ? inner_width : view.width, state->height);
    
    // Text (when selection in foreground)
    if (state->selection_in_foreground) {
        TextEdit::draw_content(state->margin, state->margin,
                               state->model, &state->measurements,
                               std::function<void(int)>());
    }
    
    // Selection
    if (has_focus(state)) {
        auto cursor_color = (state->is_mouse_dragging || caret_flicker::get_phase()) ? state->cursor_color : rgba(0x000000, 0.0);
        TextEdit::draw_selection(state->margin, state->margin,
                                 state->model, &state->measurements,
                                 cursor_color, state->selection_color);
    }
    
    // Text (when selection in background)
    if (!state->selection_in_foreground) {
        TextEdit::draw_content(state->margin, state->margin,
                               state->model, &state->measurements,
                               std::function<void(int)>());
    }
    
    // Dispose of clip region
    restore();

    // Initiate selection by mouse dragging
    if (mouse_hit(0, 0, view.width, state->height)) {
        set_cursor(CURSOR_IBEAM);
        mouse_hit_accept();
        state->is_mouse_dragging = true;
        
        float mx, my, x, y;
        mouse_position(&mx, &my);
        x = mx - state->margin + state->scroll_x;
        y = my - state->margin;
    
        locate_selection_point(&state->measurements, x, y, &state->model->selection.a_line, &state->model->selection.a_index);
        state->model->selection.b_line = state->model->selection.a_line;
        state->model->selection.b_index = state->model->selection.a_index;
        caret_flicker::reset_phase();
    }
    if (mouse_over(0, 0, view.width, state->height)) {
        set_cursor(CURSOR_IBEAM);
    }

}

}
