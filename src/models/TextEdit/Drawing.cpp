//
//  Drawing.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 19/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "Drawing.hpp"
#include "Measurements.hpp"
#include <ddui/keyboard>
#include <cstdlib>
#include <cstring>

namespace TextEdit {

void draw_content(Context ctx,
                  float offset_x, float offset_y,
                  const Model* model,
                  const Measurements* measurements,
                  std::function<void(Context,int)> update_entity) {

    float y = offset_y;
    for (int lineno = 0; lineno < model->lines.size(); ++lineno) {
    
        auto& line = model->lines[lineno];
        auto& line_measurements = measurements->lines[lineno];

        auto content = line.content.get();
        
        int i = 0;
        while (i < line.characters.size()) {
            auto& character = line.characters[i];
            auto& measurement = line_measurements.characters[i];
            
            // Handle special entity characters
            if (character.entity_id != -1) {
                auto child_ctx = child_context(ctx, offset_x + measurement.x, y + measurement.y,
                                                    measurement.width, measurement.height);
                update_entity(child_ctx, character.entity_id);
                nvgRestore(ctx.vg);
                ++i;
                continue;
            }
            
            auto style = character.style;
            
            // Get number of characters in the current segment.
            int num_chars = 1;
            while (i + num_chars < line.characters.size()) {
                auto& character = line.characters[i + num_chars];
                if (character.entity_id != -1 ||
                    memcmp(&character.style, &style, sizeof(Style)) != 0) {
                    break;
                }
                ++num_chars;
            }
            
            // Apply segment-styles
            nvgFontSize(ctx.vg, style.text_size);
            nvgFontFace(ctx.vg, style.font_bold ? model->bold_font : model->regular_font);
            nvgFillColor(ctx.vg, style.text_color);
            
            // Compose string start and end
            auto string_start = &content[character.index];
            
            auto& last_char = line.characters[i + num_chars - 1];
            auto string_end = &content[last_char.index + last_char.num_bytes];

            nvgText(ctx.vg, offset_x + measurement.x, y + measurement.baseline, string_start, string_end);
            
            i += num_chars;
        }
        
        y = offset_y + line_measurements.y + line_measurements.height;
    }
}

void draw_selection(Context ctx,
                    float offset_x, float offset_y,
                    const Model* model,
                    const Measurements* measurements,
                    NVGcolor cursor_color,
                    NVGcolor selection_color) {
    
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

}
