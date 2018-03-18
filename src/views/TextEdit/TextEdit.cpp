//
//  TextEdit.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 12/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "TextEdit.hpp"
#include "TextMeasurements.hpp"
#include <ddui/keyboard>
#include <cstdlib>

namespace TextEdit {

TextEditState::TextEditState() {
    
}

static void refresh_model(TextEditState* state, Context ctx, std::function<void(Context,int,int*,int*)> measure_entity);
static void outline_box(NVGcontext* vg, int x, int y, int width, int height);

void draw_content(Context ctx,
                  float offset_x, float offset_y,
                  const TextEditModel::Model* model,
                  const TextMeasurements::Measurements* measurements,
                  std::function<void(Context,int)> update_entity) {

    float y = offset_y;
    for (int lineno = 0; lineno < model->lines.size(); ++lineno) {
    
        auto& line = model->lines[lineno];
        auto& line_measurements = measurements->lines[lineno];

        char* content = line.content.get();
        
        for (int i = 0; i < line.characters.size(); ++i) {
            auto& character = line.characters[i];
            auto& measurement = line_measurements.characters[i];
            
            if (ctx.key->mods & keyboard::MOD_ALT) {
                outline_box(ctx.vg, offset_x + measurement.x, y + measurement.y, measurement.width, measurement.height);
                continue;
            }
            
            if (character.entity_id != -1) {
                auto child_ctx = child_context(ctx, offset_x + measurement.x, y + measurement.y,
                                                    measurement.width, measurement.height);
                update_entity(child_ctx, character.entity_id);
                nvgRestore(ctx.vg);
                continue;
            }
            

            nvgFontFace(ctx.vg, character.style.font_bold ? model->bold_font : model->regular_font);
            nvgFontSize(ctx.vg, character.style.text_size);
            nvgFillColor(ctx.vg, character.style.text_color);
            nvgText(ctx.vg, offset_x + measurement.x, y + measurement.baseline,
                            &content[character.index], &content[character.index + character.num_bytes]);
        }
        
        y = offset_y + line_measurements.y + line_measurements.height;
    }
}

void draw_selection(Context ctx,
                    float offset_x, float offset_y,
                    const TextEditModel::Model* model,
                    const TextMeasurements::Measurements* measurements,
                    NVGcolor cursor_color,
                    NVGcolor selection_color) {

    *ctx.cursor = CURSOR_IBEAM;
    
    nvgSave(ctx.vg);
    nvgTranslate(ctx.vg, offset_x, offset_y);

    auto selection = model->selection;
    
    if (selection.a_line  == selection.b_line &&
        selection.a_index == selection.b_index) {
        // Cursor
        
        float x;
        
        auto& line = measurements->lines[selection.a_line];
        if (line.characters.empty() || selection.a_index == 0) {
            x = 0.0;
        } else {
            x = line.characters[selection.a_index - 1].max_x;
        }

        nvgBeginPath(ctx.vg);
        nvgStrokeColor(ctx.vg, cursor_color);
        nvgStrokeWidth(ctx.vg, 2.0);
        nvgMoveTo(ctx.vg, x, line.y);
        nvgLineTo(ctx.vg, x, line.y + line.height);
        nvgStroke(ctx.vg);
        
    } else {
        // Selection
        
        int l1 = selection.a_line;
        int l2 = selection.b_line;
        int i1 = selection.a_index;
        int i2 = selection.b_index;
        if (l1 > l2) {
            std::swap(l1, l2);
            std::swap(i1, i2);
        }
        if (l1 == l2 && i1 > i2) {
            std::swap(i1, i2);
        }
        
        float x1, y1, height1;
        float x2, y2, height2;
        
        {
            auto& line = measurements->lines[l1];
            height1 = line.height;
            y1 = line.y;
            x1 = (i1 == 0) ? 0.0 : line.characters[i1 - 1].max_x;
        }
        
        {
            auto& line = measurements->lines[l2];
            height2 = line.height;
            y2 = line.y;
            x2 = (i2 == 0) ? 0.0 : line.characters[i2 - 1].max_x;
        }
        
        nvgBeginPath(ctx.vg);
        nvgFillColor(ctx.vg, selection_color);
        
        if (l1 == l2) {
            // Single-line selection

            float y = height1 > height2 ? y1 : y2;
            float height = height1 > height2 ? height1 : height2;
            
            nvgRect(ctx.vg, x1, y, x2 - x1, height);
            
        } else if (l2 - l1 == 1 && x1 > x2) {
            // Broken multi-line selection

            nvgRect(ctx.vg, x1, y1, ctx.width - x1 - offset_x, height1);
            nvgRect(ctx.vg, -offset_x, y2, x2 + offset_x, height2);
        } else {
            // Full multi-line selection

            nvgMoveTo(ctx.vg, x1, y1);
            nvgLineTo(ctx.vg, ctx.width - offset_x, y1);
            nvgLineTo(ctx.vg, ctx.width - offset_x, y2);
            nvgLineTo(ctx.vg, x2, y2);
            nvgLineTo(ctx.vg, x2, y2 + height2);
            nvgLineTo(ctx.vg, -offset_x, y2 + height2);
            nvgLineTo(ctx.vg, -offset_x, y1 + height1);
            nvgLineTo(ctx.vg, x1, y1 + height1);
            nvgClosePath(ctx.vg);
        }
        
        nvgFill(ctx.vg);
    }
    
    nvgRestore(ctx.vg);
}

void update(TextEditState* state,
            Context ctx,
            std::function<void(Context,int,int*,int*)> measure_entity,
            std::function<void(Context,int)> update_entity) {

    keyboard::register_focus_group(ctx, state);
    
    if (keyboard::has_key_event(ctx, state)) {
        TextEditModel::apply_keyboard_input(state->model, ctx.key);
    }

    refresh_model(state, ctx, measure_entity);

    if (!keyboard::has_focus(ctx, state) && mouse_hit(ctx, 0, 0, ctx.width, ctx.height)) {
        keyboard::focus(ctx, state);
    }
    
    if (state->is_mouse_dragging && !ctx.mouse->pressed) {
        state->is_mouse_dragging = false;
    }
    if (state->is_mouse_dragging) {
        int x = ctx.mouse->x - ctx.x;
        int y = ctx.mouse->y - ctx.y;
        locate_selection_point(&state->measurements, x, y, &state->model->selection.b_line, &state->model->selection.b_index);
    }

    // White background
    nvgBeginPath(ctx.vg);
    nvgFillColor(ctx.vg, nvgRGB(0xff, 0xff, 0xff));
    nvgRect(ctx.vg, 0, 0, state->measurements.width, state->measurements.height);
    nvgFill(ctx.vg);
    
    // Text
    draw_content(ctx, 0, 0, state->model, &state->measurements, update_entity);
    
    // Selection
    draw_selection(ctx, 0, 0, state->model, &state->measurements, nvgRGB(50, 100, 255), nvgRGBA(50, 100, 255, 100));

    if (mouse_hit(ctx, 0, 0, ctx.width, ctx.height)) {
        ctx.mouse->accepted = true;
        state->is_mouse_dragging = true;
        
        int x = ctx.mouse->x - ctx.x;
        int y = ctx.mouse->y - ctx.y;
    
        locate_selection_point(&state->measurements, x, y, &state->model->selection.a_line, &state->model->selection.a_index);
        state->model->selection.b_line = state->model->selection.a_line;
        state->model->selection.b_index = state->model->selection.a_index;
    }

}

void refresh_model(TextEditState* state, Context ctx, std::function<void(Context,int,int*,int*)> measure_entity) {
    if (state->model->version_count == state->current_version_count) {
        return; // Model up-to-date
    }

    state->measurements = TextMeasurements::measure(ctx, state->model, measure_entity);
    state->current_version_count = state->model->version_count;
}

void outline_box(NVGcontext* vg, int x, int y, int width, int height) {
    nvgBeginPath(vg);
    nvgStrokeColor(vg, nvgRGB(255, 0, 0));
    nvgRect(vg, x, y, width, height);
    nvgStroke(vg);
}

}
