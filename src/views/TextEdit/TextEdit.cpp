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

void update(TextEditState* state,
            Context ctx,
            std::function<void(Context,int,int*,int*)> measure_entity,
            std::function<void(Context,int)> update_entity) {

    keyboard::register_focus_group(ctx, state);
    
    if (keyboard::has_key_event(ctx, state)) {
        apply_keyboard_input(state->model, ctx.key);
    }

    refresh_model(state, ctx, measure_entity);

    if (!keyboard::has_focus(ctx, state) && mouse_hit(ctx, 0, 0, ctx.width, ctx.height)) {
        keyboard::focus(ctx, state);
    }

    // White background
    nvgBeginPath(ctx.vg);
    nvgFillColor(ctx.vg, nvgRGB(0xff, 0xff, 0xff));
    nvgRect(ctx.vg, 0, 0, ctx.width, ctx.height);
    nvgFill(ctx.vg);
    
    // Text
    int y = 0;
    for (int lineno = 0; lineno < state->model->lines.size(); ++lineno) {
    
        auto& line = state->model->lines[lineno];
        auto& measurements = state->measurements[lineno];

        char* content = line.content.get();
        
        for (int i = 0; i < line.characters.size(); ++i) {
            auto& character = line.characters[i];
            auto& measurement = measurements.characters[i];
            
            if (character.entity_id != -1) {
                auto child_ctx = child_context(ctx, measurement.x, y + measurement.y,
                                                    measurement.width, measurement.height);
                update_entity(child_ctx, character.entity_id);
                nvgRestore(ctx.vg);
                continue;
            }
            
            nvgFontFace(ctx.vg, character.style.font_bold ? "bold" : "regular");
            nvgFontSize(ctx.vg, character.style.text_size);
            nvgFillColor(ctx.vg, character.style.text_color);
            nvgText(ctx.vg, measurement.x, y + measurement.y,
                            &content[character.index], &content[character.index + character.num_bytes]);
        }
        
        y = measurements.y + measurements.height;
    }
    
    // Selection
    {
        *ctx.cursor = CURSOR_IBEAM;

        auto selection = state->model->selection;
        
        if (selection.a_line  == selection.b_line &&
            selection.a_index == selection.b_index) {
            // Cursor
            
            float x;
            
            auto& line = state->measurements[selection.a_line];
            if (line.characters.empty() || selection.a_index == 0) {
                x = 0.0;
            } else {
                x = line.characters[selection.a_index - 1].max_x;
            }

            if (keyboard::has_focus(ctx, state)) {
                nvgBeginPath(ctx.vg);
                nvgStrokeColor(ctx.vg, nvgRGB(50, 100, 255));
                nvgStrokeWidth(ctx.vg, 2.0);
                nvgMoveTo(ctx.vg, x, line.y);
                nvgLineTo(ctx.vg, x, line.y + line.height);
                nvgStroke(ctx.vg);
            }
            
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
                auto& line = state->measurements[l1];
                height1 = line.height;
                y1 = line.y;
                x1 = (i1 == 0) ? 0.0 : line.characters[i1 - 1].max_x;
            }
            
            {
                auto& line = state->measurements[l2];
                height2 = line.height;
                y2 = line.y;
                x2 = (i2 == 0) ? 0.0 : line.characters[i2 - 1].max_x;
            }
            
            nvgBeginPath(ctx.vg);
            nvgFillColor(ctx.vg, nvgRGBA(50, 100, 255, 100));
            
            if (l1 == l2) {
                // Single-line selection

                float y = height1 > height2 ? y1 : y2;
                float height = height1 > height2 ? height1 : height2;
                
                nvgRect(ctx.vg, x1, y, x2 - x1, height);
                
            } else if (l2 - l1 == 1 && x1 > x2) {
                // Broken multi-line selection

                nvgRect(ctx.vg, x1, y1, ctx.width - x1, height1);
                nvgRect(ctx.vg, 0, y2, x2, height2);
            } else {
                // Full multi-line selection

                nvgMoveTo(ctx.vg, x1, y1);
                nvgLineTo(ctx.vg, ctx.width, y1);
                nvgLineTo(ctx.vg, ctx.width, y2);
                nvgLineTo(ctx.vg, x2, y2);
                nvgLineTo(ctx.vg, x2, y2 + height2);
                nvgLineTo(ctx.vg, 0, y2 + height2);
                nvgLineTo(ctx.vg, 0, y1 + height1);
                nvgLineTo(ctx.vg, x1, y1 + height1);
                nvgClosePath(ctx.vg);
            }
            
            nvgFill(ctx.vg);
        }
    }

    if (mouse_hit(ctx, 0, 0, ctx.width, ctx.height)) {
        ctx.mouse->accepted = true;
    }

}

void refresh_model(TextEditState* state, Context ctx, std::function<void(Context,int,int*,int*)> measure_entity) {
    if (state->model->version_count == state->current_version_count) {
        return; // Model up-to-date
    }

    float y = 0.0;

    state->measurements.clear();
    state->measurements.reserve(state->model->lines.size());
    for (auto& line : state->model->lines) {
        auto measurements = measure(ctx, &line, 20.0, "regular", "bold", measure_entity);
        measurements.y = y;
        y += measurements.height;
        state->measurements.push_back(std::move(measurements));
    }

    state->current_version_count = state->model->version_count;
}

void outline_box(NVGcontext* vg, int x, int y, int width, int height) {
    nvgBeginPath(vg);
    nvgStrokeColor(vg, nvgRGB(255, 0, 0));
    nvgRect(vg, x, y, width, height);
    nvgStroke(vg);
}

}
