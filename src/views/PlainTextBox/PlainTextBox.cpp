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

using namespace ddui;

PlainTextBox::StyleOptions* PlainTextBox::get_global_styles() {
    static StyleOptions styles;
    static bool did_init = false;
    if (did_init) {
        return &styles;
    }

    did_init = true;
    styles.margin = 8;
    styles.border_radius = 4;
    styles.border_width = 1;
    styles.border_color = ddui::rgb(0xc8c8c8);
    styles.border_color_focused = ddui::rgb(0x3264ff);
    styles.bg_color = ddui::rgb(0xffffff);
    styles.bg_color_focused = ddui::rgb(0xffffff);
    styles.cursor_color = ddui::rgb(0x3264ff);
    styles.selection_color = ddui::rgba(0x3264ff, 0.4);
    styles.selection_in_foreground = true;
    return &styles;
}

PlainTextBox::PlainTextBox(State* _state, Model* _model)
    : state(*_state), model(*_model) {

    if (state.model != &model) {
        state.model = &model;
        state.current_version_count = -1;
    }

    styles = get_global_styles();
    multiline = false;
}

PlainTextBox& PlainTextBox::set_styles(const StyleOptions* styles) {
    this->styles = styles;
    return *this;
}

PlainTextBox& PlainTextBox::set_multiline(bool multiline) {
    this->multiline = multiline;
    return *this;
}

void PlainTextBox::update() {

    const auto group_id = (const void*)&state;

    FocusItem { group_id };
    
    // Process key input
    if (has_key_event(group_id)) {
        process_key_input();
    }

    // When tabbing in to focus, select the entire content
    if (did_focus(group_id) && !state.is_mouse_dragging) {
        auto& selection = model.selection;
        if (selection.a_line  == selection.b_line &&
            selection.a_index == selection.b_index) {
            selection.a_line = 0;
            selection.a_index = 0;
            selection.b_line = model.lines.size() - 1;
            selection.b_index = model.lines.back().characters.size();
            model.version_count++;
        }
    }
    
    // Reset the flickering caret to ON whenever the model has changed
    if (has_focus(group_id) && state.current_version_count != model.version_count) {
        caret_flicker::reset_phase();
    }

    // Refresh the model measurements
    if (model.version_count != state.current_version_count) {
        refresh_model_measurements();
        state.current_version_count = model.version_count;
    }

    // Update the text box height
    state.height = state.measurements.height + 2 * styles->margin;

    // Focus the box on a mouse click
    if (!has_focus(group_id) && mouse_hit(0, 0, view.width, state.height)) {
        focus(group_id);
        caret_flicker::reset_phase();
    }
    
    // Update selection by mouse dragging (it's initiated at the end of this function)
    if (state.is_mouse_dragging && !mouse_state.pressed) {
        state.is_mouse_dragging = false;
    }
    if (state.is_mouse_dragging) {
        set_cursor(CURSOR_IBEAM);

        float mx, my, x, y;
        mouse_position(&mx, &my);
        x = mx - styles->margin + state.scroll_x;
        y = my - styles->margin;
        
        TextEdit::locate_selection_point(&state.measurements, x, y,
                                         &model.selection.b_line,
                                         &model.selection.b_index);
    }

    // Background
    begin_path();
    fill_color(has_focus(group_id) ? styles->bg_color_focused : styles->bg_color);
    rounded_rect(0, 0, view.width, state.height, styles->border_radius);
    fill();
    
    // Border
    begin_path();
    stroke_color(has_focus(group_id) ? styles->border_color_focused : styles->border_color);
    stroke_width(styles->border_width);
    rounded_rect(0, 0, view.width, state.height, styles->border_radius);
    stroke();
    
    // Update scrolling
    int caret_x = (model.selection.b_index == 0 ? 0 :
                   state.measurements.lines[model.selection.b_line]
                        .characters[model.selection.b_index - 1].max_x);
    while (caret_x < state.scroll_x) {
        state.scroll_x -= 50;
    }
    while (caret_x > state.scroll_x + view.width - 2 * styles->margin) {
        state.scroll_x += 50;
    }
    if (state.scroll_x > state.measurements.width - view.width + 2 * styles->margin) {
        state.scroll_x = state.measurements.width - view.width + 2 * styles->margin;
    }
    if (state.scroll_x < 0) {
        state.scroll_x = 0;
    }
    
    // Prepare clip-region
    auto inner_width = state.measurements.width + 2 * styles->margin;
    save();
    clip(0, 0, view.width, state.height);
    sub_view(-state.scroll_x, 0, inner_width > view.width ? inner_width : view.width, state.height);
    
    // Text (when selection in foreground)
    if (styles->selection_in_foreground) {
        draw_content();
    }
    
    // Selection
    if (has_focus(group_id)) {
        auto cursor_color = (state.is_mouse_dragging || caret_flicker::get_phase()) ? styles->cursor_color : rgba(0x000000, 0.0);
        draw_selection(cursor_color);
    }
    
    // Text (when selection in background)
    if (!styles->selection_in_foreground) {
        draw_content();
    }
    
    // Dispose of clip region
    restore();
    restore();

    // Initiate selection by mouse dragging
    if (mouse_hit(0, 0, view.width, state.height)) {
        set_cursor(CURSOR_IBEAM);
        mouse_hit_accept();
        state.is_mouse_dragging = true;
        
        float mx, my, x, y;
        mouse_position(&mx, &my);
        x = mx - styles->margin + state.scroll_x;
        y = my - styles->margin;
    
        locate_selection_point(&state.measurements, x, y, &model.selection.a_line, &model.selection.a_index);
        model.selection.b_line = model.selection.a_line;
        model.selection.b_index = model.selection.a_index;
        caret_flicker::reset_phase();
    }
    if (mouse_over(0, 0, view.width, state.height)) {
        set_cursor(CURSOR_IBEAM);
    }

}

void PlainTextBox::process_key_input() {
    if (multiline ||
        (key_state.key != keyboard::KEY_ENTER &&
         key_state.key != keyboard::KEY_KP_ENTER)) {
        TextEdit::apply_keyboard_input(&model, &key_state);
    }
    if (!multiline) {
        TextEdit::remove_line_breaks(&model);
    }
}

void PlainTextBox::refresh_model_measurements() {
    using namespace std::placeholders;
    auto measure_entity_fn = std::bind(&PlainTextBox::measure_entity, this, _1, _2, _3, _4, _5);
    state.measurements = TextEdit::measure(&model, measure_entity_fn);
}

void PlainTextBox::measure_entity(int line, int index, int entity_id, float* width, float* height) {
    // NO-OP
}

void PlainTextBox::draw_entity(int line, int index, int entity_id) {
    // NO-OP
}

void PlainTextBox::draw_content() {
    using namespace std::placeholders;
    auto draw_entity_fn = std::bind(&PlainTextBox::draw_entity, this, _1, _2, _3);
    TextEdit::draw_content(styles->margin, styles->margin,
                           &model, &state.measurements,
                           draw_entity_fn);
}

void PlainTextBox::draw_selection(const ddui::Color& cursor_color) {
    TextEdit::draw_selection(styles->margin, styles->margin,
                             &model, &state.measurements,
                             cursor_color, styles->selection_color);
}
