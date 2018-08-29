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
  
    if (width <= 0) {
        dst[0] = '\0';
        return 0;
    }
  
    float bounds[4];
  
    strcpy(dst, src);
    ddui::text_bounds(0, 0, src, 0, bounds);
    auto text_width = bounds[2] - bounds[0];
  
    if (width >= text_width) {
        return text_width;
    }
  
    int N = strlen - 1;
  
    strcpy(dst, src);
    dst[N    ] = '.';
    dst[N + 1] = '.';
    dst[N + 2] = '.';
    dst[N + 3] = '\0';
    ddui::text_bounds(0, 0, dst, 0, bounds);
    text_width = bounds[2] - bounds[0];
  
    while (width < text_width && N > 0) {
        --N;
        dst[N] = '.';
        dst[N + 3] = '\0';
        ddui::text_bounds(0, 0, dst, 0, bounds);
        text_width = bounds[2] - bounds[0];
    }
  
    while (width < text_width && N > -3) {
        --N;
        dst[N + 3] = '\0';
        ddui::text_bounds(0, 0, dst, 0, bounds);
        text_width = bounds[2] - bounds[0];
    }
  
    return width;
}
