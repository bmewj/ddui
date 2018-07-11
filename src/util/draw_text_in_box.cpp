//
//  draw_text_in_box.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 11/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "draw_text_in_box.hpp"
#include <string.h>

void draw_text_in_box(NVGcontext* vg, int x, int y, int width, int height, const char* content) {
    constexpr int MARGIN = 2;
  
    int length = strlen(content);
    char new_content[length + 4];
    truncate_text(vg, width - 2 * MARGIN, length, new_content, content);
  
    float ascender, descender, line_height;
    nvgTextMetrics(vg, &ascender, &descender, &line_height);
  
    int text_x = x + MARGIN;
    int text_y = y + (height - (int)line_height) / 2 + (int)ascender;

    nvgText(vg, text_x, text_y, new_content, 0);
}

void draw_centered_text_in_box(NVGcontext* vg, int x, int y, int width, int height, const char* content) {
    constexpr int MARGIN = 2;
  
    int length = strlen(content);
    char new_content[length + 4];
    int text_width = truncate_text(vg, width - 2 * MARGIN, length, new_content, content);
  
    float ascender, descender, line_height;
    nvgTextMetrics(vg, &ascender, &descender, &line_height);
  
    int text_x = x + (width - text_width) / 2 + MARGIN;
    int text_y = y + (height - (int)line_height) / 2 + (int)ascender;

    nvgText(vg, text_x, text_y, new_content, 0);
}

int truncate_text(NVGcontext* vg, int width, int strlen, char* dst, const char* src) {
  
    if (width <= 0) {
        dst[0] = '\0';
        return 0;
    }
  
    float bounds[4];
  
    strcpy(dst, src);
    nvgTextBounds(vg, 0, 0, src, 0, bounds);
    int text_width = bounds[2] - bounds[0];
  
    if (width >= text_width) {
        return text_width;
    }
  
    int N = strlen - 1;
  
    strcpy(dst, src);
    dst[N    ] = '.';
    dst[N + 1] = '.';
    dst[N + 2] = '.';
    dst[N + 3] = '\0';
    nvgTextBounds(vg, 0, 0, dst, 0, bounds);
    text_width = bounds[2] - bounds[0];
  
    while (width < text_width && N > 0) {
        --N;
        dst[N] = '.';
        dst[N + 3] = '\0';
        nvgTextBounds(vg, 0, 0, dst, 0, bounds);
        text_width = bounds[2] - bounds[0];
    }
  
    while (width < text_width && N > -3) {
        --N;
        dst[N + 3] = '\0';
        nvgTextBounds(vg, 0, 0, dst, 0, bounds);
        text_width = bounds[2] - bounds[0];
    }
  
    return width;
}
