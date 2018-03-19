//
//  RichTextBox.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 19/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "RichTextBox.hpp"
#include <ddui/views/TextEdit>
#include <ddui/keyboard>
#include "../TextEdit/TextEditCaret.hpp"
#include <cstdlib>

namespace RichTextBox {

RichTextBoxState::RichTextBoxState() {
    current_version_count = -1;
}

static void apply_rich_text_commands(TextEditModel::Model* model, KeyState* key);

void update(RichTextBoxState* state, Context ctx) {

    keyboard::register_focus_group(ctx, state);
    
    // Process key input
    if (keyboard::has_key_event(ctx, state)) {
        if (state->multiline || ctx.key->key != keyboard::KEY_ENTER) {
            TextEditModel::apply_keyboard_input(state->model, ctx.key);
        }
        apply_rich_text_commands(state->model, ctx.key);
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
        TextEditCaret::reset_phase();
    }

    // Refresh the model measurements
    if (state->model->version_count != state->current_version_count) {
        state->measurements = TextMeasurements::measure(ctx, state->model, std::function<void(Context,int,int*,int*)>());
        state->current_version_count = state->model->version_count;
    }
    
    // Update the text box height
    state->height = state->measurements.height + 2 * state->margin;

    // Focus the box on a mouse click
    if (!keyboard::has_focus(ctx, state) && mouse_hit(ctx, 0, 0, ctx.width, state->height)) {
        keyboard::focus(ctx, state);
        TextEditCaret::reset_phase();
    }
    
    // Update selection by mouse dragging (it's initiated at the end of this function)
    if (state->is_mouse_dragging && !ctx.mouse->pressed) {
        state->is_mouse_dragging = false;
    }
    if (state->is_mouse_dragging) {
        *ctx.cursor = CURSOR_IBEAM;
        int x = ctx.mouse->x - ctx.x - state->margin + state->scroll_x;
        int y = ctx.mouse->y - ctx.y - state->margin;
        TextMeasurements::locate_selection_point(&state->measurements, x, y,
                                                 &state->model->selection.b_line,
                                                 &state->model->selection.b_index);
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
    int caret_x = (state->model->selection.b_index == 0 ? 0 :
                   state->measurements.lines[state->model->selection.b_line]
                                 .characters[state->model->selection.b_index - 1].max_x);
    while (caret_x < state->scroll_x) {
        state->scroll_x -= 50;
    }
    while (caret_x > state->scroll_x + ctx.width - 2 * state->margin) {
        state->scroll_x += 50;
    }
    if (state->scroll_x > state->measurements.width - ctx.width + 2 * state->margin) {
        state->scroll_x = state->measurements.width - ctx.width + 2 * state->margin;
    }
    if (state->scroll_x < 0) {
        state->scroll_x = 0;
    }
    
    // Prepare clip-region
    nvgSave(ctx.vg);
    nvgScissor(ctx.vg, 0, 0, ctx.width, state->height);
    nvgTranslate(ctx.vg, -state->scroll_x, 0);
    
    auto inner_width = state->measurements.width + 2 * state->margin;
    
    auto child_ctx = ctx;
    child_ctx.x -= state->scroll_x;
    child_ctx.width = inner_width > ctx.width ? inner_width : ctx.width;
    child_ctx.clip.x1 = ctx.x;
    child_ctx.clip.y1 = ctx.y;
    child_ctx.clip.x2 = ctx.x + ctx.width;
    child_ctx.clip.y2 = ctx.y + state->height;
    
    // Text (when selection in foreground)
    if (state->selection_in_foreground) {
        TextEdit::draw_content(child_ctx,
                               state->margin, state->margin,
                               state->model, &state->measurements,
                               std::function<void(Context,int)>());
    }
    
    // Selection
    if (keyboard::has_focus(ctx, state)) {
        auto cursor_color = (state->is_mouse_dragging || TextEditCaret::get_phase()) ? state->cursor_color : nvgRGBA(0, 0, 0, 0);
        TextEdit::draw_selection(child_ctx,
                                 state->margin, state->margin,
                                 state->model, &state->measurements,
                                 cursor_color, state->selection_color);
    }
    
    // Text (when selection in background)
    if (!state->selection_in_foreground) {
        TextEdit::draw_content(child_ctx,
                               state->margin, state->margin,
                               state->model, &state->measurements,
                               std::function<void(Context,int)>());
    }
    
    // Dispose of clip region
    nvgRestore(ctx.vg);

    // Initiate selection by mouse dragging
    if (mouse_hit(ctx, 0, 0, ctx.width, state->height)) {
        *ctx.cursor = CURSOR_IBEAM;
        ctx.mouse->accepted = true;
        state->is_mouse_dragging = true;
        
        int x = ctx.mouse->x - ctx.x - state->margin + state->scroll_x;
        int y = ctx.mouse->y - ctx.y - state->margin;
    
        locate_selection_point(&state->measurements, x, y, &state->model->selection.a_line, &state->model->selection.a_index);
        state->model->selection.b_line = state->model->selection.a_line;
        state->model->selection.b_index = state->model->selection.a_index;
        TextEditCaret::reset_phase();
    }
    if (mouse_over(ctx, 0, 0, ctx.width, state->height)) {
        *ctx.cursor = CURSOR_IBEAM;
    }

}

void apply_rich_text_commands(TextEditModel::Model* model, KeyState* key) {

    if (key->action != keyboard::ACTION_PRESS) {
        return;
    }

    auto& sel = model->selection;

    auto min_line  = sel.a_line < sel.b_line ? sel.a_line  : sel.b_line;
    auto min_index = sel.a_line < sel.b_line ? sel.a_index : sel.b_index;
    if (sel.a_line == sel.b_line) {
        min_index = sel.a_index < sel.b_index ? sel.a_index : sel.b_index;
    }

    if ((key->mods & keyboard::MOD_SUPER) &&
        (key->key == keyboard::KEY_B)) {
        
        auto& line = model->lines[min_line];
        if (min_index < line.characters.size()) {
            TextEditModel::StyleCommand style;
            style.type = TextEditModel::StyleCommand::BOLD;
            style.bool_value = !line.characters[min_index].style.font_bold;
            TextEditModel::apply_style(model, sel, style);
        }
    }

    if ((key->mods & keyboard::MOD_SUPER) &&
        (key->mods & keyboard::MOD_SHIFT) &&
        (key->key == keyboard::KEY_EQUAL)) {
        
        auto& line = model->lines[min_line];
        if (min_index < line.characters.size()) {
            TextEditModel::StyleCommand style;
            style.type = TextEditModel::StyleCommand::SIZE;
            style.float_value = line.characters[min_index].style.text_size * 1.1;
            TextEditModel::apply_style(model, sel, style);
        }
    }

    if ((key->mods & keyboard::MOD_SUPER) &&
        (key->key == keyboard::KEY_MINUS)) {
        
        auto& line = model->lines[min_line];
        if (min_index < line.characters.size()) {
            TextEditModel::StyleCommand style;
            style.type = TextEditModel::StyleCommand::SIZE;
            style.float_value = line.characters[min_index].style.text_size * (1 / 1.1);
            TextEditModel::apply_style(model, sel, style);
        }
    }
}

}
