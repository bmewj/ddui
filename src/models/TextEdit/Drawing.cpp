//
//  Drawing.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 19/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "Drawing.hpp"
#include "Measurements.hpp"
#include <cstdlib>
#include <cstring>

namespace TextEdit {

using namespace ddui;

void draw_content(float offset_x, float offset_y,
                  const Model* model,
                  const Measurements* measurements,
                  std::function<void(int)> update_entity) {

    text_align(align::LEFT | align::BASELINE);

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
                sub_view(offset_x + measurement.x, y + measurement.y,
                         measurement.width, measurement.height);
                update_entity(character.entity_id);
                restore();
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
            font_size(style.text_size);
            font_face(style.font_bold ? model->bold_font : model->regular_font);
            fill_color(style.text_color);
            
            // Compose string start and end
            auto string_start = &content[character.index];
            
            auto& last_char = line.characters[i + num_chars - 1];
            auto string_end = &content[last_char.index + last_char.num_bytes];

            text(offset_x + measurement.x, y + measurement.baseline, string_start, string_end);
            
            i += num_chars;
        }
        
        y = offset_y + line_measurements.y + line_measurements.height;
    }
}

void draw_selection(float offset_x, float offset_y,
                    const Model* model,
                    const Measurements* measurements,
                    Color cursor_color,
                    Color selection_color) {
    
    save();
    translate(offset_x, offset_y);

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

        begin_path();
        stroke_color(cursor_color);
        stroke_width(2.0);
        move_to(x, line.y);
        line_to(x, line.y + line.height);
        stroke();
        
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
        
        begin_path();
        fill_color(selection_color);
        
        if (l1 == l2) {
            // Single-line selection

            float y = height1 > height2 ? y1 : y2;
            float height = height1 > height2 ? height1 : height2;
            
            rect(x1, y, x2 - x1, height);
            
        } else if (l2 - l1 == 1 && x1 > x2) {
            // Broken multi-line selection

            rect(x1, y1, view.width - x1 - offset_x, height1);
            rect(-offset_x, y2, x2 + offset_x, height2);
        } else {
            // Full multi-line selection

            move_to(x1, y1);
            line_to(view.width - offset_x, y1);
            line_to(view.width - offset_x, y2);
            line_to(x2, y2);
            line_to(x2, y2 + height2);
            line_to(-offset_x, y2 + height2);
            line_to(-offset_x, y1 + height1);
            line_to(x1, y1 + height1);
            fill();
        }
        
        fill();
    }
    
    restore();
}

}
