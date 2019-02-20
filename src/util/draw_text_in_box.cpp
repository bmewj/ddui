//
//  draw_text_in_box.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 11/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "draw_text_in_box.hpp"
#include <ddui/core>
#include <string.h>

void draw_text_in_box(float x, float y, float width, float height, const char* content) {
    constexpr float MARGIN = 2.0;
  
    int length = strlen(content);
    char new_content[length + 4];
    truncate_text(width - 2 * MARGIN, length, new_content, content);
  
    float ascender, descender, line_height;
    ddui::text_metrics(&ascender, &descender, &line_height);
  
    auto text_x = x + MARGIN;
    auto text_y = y + (height - line_height) / 2 + ascender;
    ddui::text(text_x, text_y, new_content, 0);
}

void draw_centered_text_in_box(float x, float y, float width, float height, const char* content) {
    constexpr float MARGIN = 2.0;
  
    int length = strlen(content);
    char new_content[length + 4];
    auto text_width = truncate_text(width - 2 * MARGIN, length, new_content, content);
  
    float ascender, descender, line_height;
    ddui::text_metrics(&ascender, &descender, &line_height);
  
    auto text_x = x + (width - text_width) / 2 + MARGIN;
    auto text_y = y + (height - line_height) / 2 + ascender;
    ddui::text(text_x, text_y, new_content, 0);
}

int truncate_text(float width, int strlen, char* dst, const char* src) {

    // Handle 0 width case
    if (width <= 0 || strlen == 0) {
        dst[0] = '\0';
        return 0;
    }

    // Measure width of ...
    float bounds[4];
    ddui::text_bounds(0, 0, "...", NULL, bounds);
    auto ellipses_width = bounds[2] - bounds[0];
    
    // Handle less than ... width case
    if (width <= ellipses_width) {
        dst[0] = '\0';
        return 0;
    }
    
    // Get glyph positions
    ddui::GlyphPosition glyph_positions[strlen];
    auto num_glyph_positions = ddui::text_glyph_positions(0, 0, src, NULL, glyph_positions, strlen);
    
    strcpy(dst, src);
    
    // Does the entire line fit?
    auto text_width = glyph_positions[num_glyph_positions - 1].maxx - glyph_positions[0].minx;
    if (text_width <= width) {
        return text_width + 2;
    }
    
    // How much can we fit?
    auto num_chars = num_glyph_positions;
    for (; num_chars > 0; --num_chars) {
        text_width = glyph_positions[num_chars - 1].maxx - glyph_positions[0].minx + ellipses_width;
        if (text_width <= width) {
            break;
        }
    }
    
    // Append the dots
    dst[num_chars] = '.';
    dst[num_chars+1] = '.';
    dst[num_chars+2] = '.';
    dst[num_chars+3] = '\0';
    return text_width + 2;
}
