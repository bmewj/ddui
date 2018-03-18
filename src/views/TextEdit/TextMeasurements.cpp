//
//  TextMeasurements.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 13/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "TextMeasurements.hpp"

namespace TextMeasurements {

Measurements measure(Context ctx,
                     const TextEditModel::Model* model,
                     std::function<void(Context,int,int*,int*)> measure_entity) {

    float y = 0.0;

    Measurements output;
    output.lines.reserve(model->lines.size());

    for (auto& line : model->lines) {
        auto measurements = measure(ctx, model, &line, measure_entity);
        measurements.y = y;
        y += measurements.height;
        if (!measurements.characters.empty() && output.width < measurements.characters.back().max_x) {
            output.width = measurements.characters.back().max_x;
        }
        output.lines.push_back(std::move(measurements));
    }

    output.height = y;
    return output;
}

LineMeasurements measure(Context ctx,
                         const TextEditModel::Model* model,
                         const TextEditModel::Line* line,
                         std::function<void(Context,int,int*,int*)> measure_entity) {
    
    auto& characters = line->characters;
    char* content = line->content.get();

    auto regular_font = model->regular_font;
    auto bold_font    = model->bold_font;
    
    LineMeasurements output;
    output.line_height = 0.0;
    output.height = 0.0;
    output.max_ascender = 0.0;
    
    output.characters.reserve(characters.size());
    Character empty_character = { 0 };
    for (int i = 0; i < characters.size(); ++i) {
        output.characters.push_back(empty_character);
    }
    
    float ascender, descender, line_height;

    if (characters.empty()) {
        nvgFontSize(ctx.vg, line->style.text_size);
        nvgFontFace(ctx.vg, line->style.font_bold ? bold_font : regular_font);
        nvgTextMetrics(ctx.vg, &ascender, &descender, &line_height);
        
        output.line_height = output.height = line_height;
        output.max_ascender = ascender;
        return output;
    }
    
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
            output.characters[i].width = width;
            output.characters[i].height = height;
            output.characters[i].max_x = x + width;
            output.characters[i].line_height = height;
            output.characters[i].ascender = 0.0;
            
            if (output.height < height) {
                output.height = height;
            }
            
            x += width;
            ++i;
            continue;
        }
        
        auto font_bold = characters[i].style.font_bold;
        auto text_size = characters[i].style.text_size;
        
        // Get number of characters in the current segment.
        int num_chars = 1;
        while (i + num_chars < characters.size()) {
            auto& character = characters[i + num_chars];
            if (character.entity_id != -1 ||
                character.style.font_bold != font_bold ||
                character.style.text_size != text_size) {
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
            measurement.width = positions[j].maxx - positions[j].x;
            measurement.height = line_height;
            measurement.max_x = x + positions[j].maxx;
            measurement.line_height = line_height;
            measurement.ascender = ascender;
        }
        
        x += positions[num_chars - 1].maxx;
        i += num_chars;
    }
    
    // Second pass: update baselines for all characters
    float baseline = (output.height - output.line_height) / 2 + output.max_ascender;
    
    for (int i = 0; i < characters.size(); ++i) {
        auto& measurement = output.characters[i];
        measurement.baseline = baseline;
        if (characters[i].entity_id == -1) {
            measurement.y = baseline - measurement.ascender;
        } else if (measurement.height < output.max_ascender) {
            measurement.y = baseline - measurement.height;
        } else {
            measurement.y = (output.height - measurement.height) / 2;
        }
    }
    
    return output;
}

void locate_selection_point(const Measurements* measurements, int x, int y, int* lineno, int* index) {

    auto& lines = measurements->lines;

    if (y < 0.0) {
        *lineno = 0;
        *index = 0;
        return;
    }
    
    if (y >= lines.back().y + lines.back().height) {
        *lineno = lines.size() - 1;
        *index = lines.back().characters.size();
        return;
    }
    
    for (int i = 0; i < lines.size(); ++i) {
        if (y < lines[i].y + lines[i].height) {
            *lineno = i;
            break;
        }
    }
    
    auto& line = lines[*lineno];
    
    float prev_max_x = 0.0;
    
    for (int i = 0; i < line.characters.size(); ++i) {
        float mid = (prev_max_x + line.characters[i].max_x) / 2;
        if (x < mid) {
            *index = i;
            return;
        }
        prev_max_x = line.characters[i].max_x;
    }
    
    *index = line.characters.size();
}

}
