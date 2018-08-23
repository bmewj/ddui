//
//  PlainTextBox.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 18/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_PlainTextBox_hpp
#define ddui_PlainTextBox_hpp

#include <ddui/ddui>
#include <ddui/models/TextEdit>
#include <string>
#include <vector>

namespace PlainTextBox {

struct PlainTextBoxState {
    PlainTextBoxState();

    TextEdit::Model* model;
    bool multiline = false;

    // Text edit state
    int current_version_count;
    TextEdit::Measurements measurements;

    // UI info
    bool is_mouse_dragging;
    float height;
    float scroll_x;
    
    // Styles
    float margin = 8;
    float border_radius = 4;
    float border_width = 1;
    ddui::Color border_color = ddui::rgb(200, 200, 200);
    ddui::Color border_color_focused = ddui::rgb(50, 100, 255);
    ddui::Color bg_color = ddui::rgb(255, 255, 255);
    ddui::Color bg_color_focused = ddui::rgb(255, 255, 255);
    ddui::Color cursor_color = ddui::rgb(50, 100, 255);
    ddui::Color selection_color = ddui::rgba(50, 100, 255, 100);
    bool selection_in_foreground = true;
};

void update(PlainTextBoxState* state);

}

#endif
