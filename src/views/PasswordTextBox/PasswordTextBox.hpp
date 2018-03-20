//
//  PasswordTextBox.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 20/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_PasswordTextBox_hpp
#define ddui_PasswordTextBox_hpp

#include <ddui/Context>
#include <ddui/models/TextEdit>
#include <string>
#include <vector>

namespace PasswordTextBox {

struct PasswordTextBoxState {
    PasswordTextBoxState();

    TextEdit::Model* model;

    // Text edit state
    int current_version_count;
    TextEdit::Measurements measurements;

    // UI info
    bool is_mouse_dragging;
    int height;
    int scroll_x;
    
    // Styles
    float margin = 8;
    float border_radius = 4;
    float border_width = 1;
    NVGcolor border_color = nvgRGB(200, 200, 200);
    NVGcolor border_color_focused = nvgRGB(50, 100, 255);
    NVGcolor bg_color = nvgRGB(255, 255, 255);
    NVGcolor bg_color_focused = nvgRGB(255, 255, 255);
    NVGcolor cursor_color = nvgRGB(50, 100, 255);
    NVGcolor selection_color = nvgRGBA(50, 100, 255, 100);
    bool selection_in_foreground = true;
};

void update(PasswordTextBoxState* state, Context ctx);

}

#endif
