//
//  TextMeasurements.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 13/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "TextMeasurements.hpp"

namespace TextEdit {

LineMeasurements measure(Context ctx,
                         const TextEditModel::Line* line,
                         float min_line_height,
                         const char* regular_font,
                         const char* bold_font,
                         std::function<void(Context,int,int*,int*)> measure_entity) {
    
    auto& characters = line->characters;
    char* content = line->content.get();
    
    LineMeasurements output;
    output.line_height = min_line_height;
    output.height = min_line_height;
    output.max_ascender = 0.0;
    
    output.characters.reserve(characters.size());
    LineMeasurements::Character empty_character = { 0 };
    for (int i = 0; i < characters.size(); ++i) {
        output.characters.push_back(empty_character);
    }
    
    float ascender, descender, line_height;
    
    float x = 0.0;
    
    // First pass: measure all horizontal, and get maximum verticals
    int i = 0;
    while (i < characters.size()) {
        // Handle special entity characters.
        if (characters[i].entity_id != -1) {
            int width, height;
            measure_entity(ctx, characters[i].entity_id, &width, &height);
            
            // Save so that we only call measure_entity() once.
            output.characters[i].x = x;
            output.characters[i].max_x = x + width;
            output.characters[i].line_height = height;
            output.characters[i].width = width;
            output.characters[i].height = height;
            
            if (output.height < height) {
                output.height = height;
            }
            
            x += width;
            ++i;
            continue;
        }
        
        auto font_bold = characters[i].font_bold;
        auto text_size = characters[i].text_size;
        
        // Get number of characters in the current segment.
        int num_chars = 1;
        while (i + num_chars < characters.size()) {
            auto& character = characters[i + num_chars];
            if (character.entity_id != -1 ||
                character.font_bold != font_bold ||
                character.text_size != text_size) {
                break;
            }
            ++num_chars;
        }
        
        // Apply segment-styles
        nvgFontSize(ctx.vg, text_size);
        nvgFontFace(ctx.vg, font_bold ? bold_font : regular_font);
        nvgTextMetrics(ctx.vg, &ascender, &descender, &line_height);
        
        // Update vertical measurements
        if (output.line_height < line_height) {
            output.line_height = line_height;
        }
        if (output.max_ascender < ascender) {
            output.max_ascender = ascender;
        }
        if (output.height < output.line_height) {
            output.height = output.line_height;
        }
        
        // Get all the glyph positions
        auto& first_char = characters[i];
        auto& last_char = characters[i + num_chars - 1];

        auto string_start = &content[first_char.index];
        auto string_end = &content[last_char.index + last_char.num_bytes];

        NVGglyphPosition positions[num_chars];
        nvgTextGlyphPositions(ctx.vg, 0, 0, string_start, string_end, positions, num_chars);

        // Measure all the characters in segment
        for (int j = 0; j < num_chars; ++j) {
            auto& measurement = output.characters[i + j];
            measurement.x = x + positions[j].x;
            measurement.max_x = x + positions[j].maxx;
            measurement.line_height = line_height;
            measurement.width = positions[j].maxx - positions[j].x;
            measurement.height = ascender;
        }
        
        x += positions[num_chars - 1].maxx;
        i += num_chars;
    }
    
    // Second pass: update baselines for all characters
    float baseline = (output.height - output.max_ascender) / 2 + output.max_ascender;
    
    for (int i = 0; i < characters.size(); ++i) {
        auto& measurement = output.characters[i];
        if (characters[i].entity_id == -1) {
            measurement.y = baseline;
        } else if (measurement.height < output.max_ascender) {
            measurement.y = baseline - measurement.height;
        } else {
            measurement.y = (output.height - measurement.height) / 2;
        }
    }
    
    return output;
}

}
