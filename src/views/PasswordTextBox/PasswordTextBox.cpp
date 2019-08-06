//
//  PasswordTextBox.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 20/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "PasswordTextBox.hpp"

using namespace ddui;

void PasswordTextBox::refresh_model_measurements() {
    float ascender, descender, line_height;
    ddui::font_face(model.regular_font);
    ddui::font_size(model.lines.front().style.text_size);
    ddui::text_metrics(&ascender, &descender, &line_height);

    float dot_bounding_height = line_height;
    float dot_bounding_width  = 0.6 * dot_bounding_height;
    int   num_dots            = (int)model.lines.front().characters.size();
    float total_width         = num_dots * dot_bounding_width;

    state.measurements.width  = total_width;
    state.measurements.height = dot_bounding_height;
    state.measurements.lines.clear();
    state.measurements.lines.push_back(TextEdit::LineMeasurements());
    
    auto& line = state.measurements.lines.front();
    line.line_height = dot_bounding_height;
    line.height = dot_bounding_height;
    line.y = 0.0;
    line.max_ascender = ascender;
    
    float x = 0.0;

    for (int i = 0; i < num_dots; ++i) {
        line.characters.push_back(TextEdit::CharacterMeasurements());

        auto& ch = line.characters.back();
        ch.ascender = ascender;
        ch.baseline = 0.0;
        ch.height = line_height;
        ch.line_height = line_height;
        ch.width = dot_bounding_width;
        ch.x = x;
        x += dot_bounding_width;
        ch.max_x = x;
    }
}

void PasswordTextBox::draw_content() {
    float dot_bounding_height = state.measurements.height;
    float dot_bounding_width  = 0.6 * dot_bounding_height;
    float dot_radius          = 0.4 * dot_bounding_width;

    float x = 0.5 * dot_bounding_width + styles->margin;
    float y = 0.5 * dot_bounding_height + styles->margin;

    int num_dots = model.lines.front().characters.size();

    ddui::fill_color(model.lines.front().style.text_color);
    for (int i = 0; i < num_dots; ++i) {
        ddui::begin_path();
        ddui::circle(x, y, dot_radius);
        ddui::fill();
        x += dot_bounding_width;
    }
}
