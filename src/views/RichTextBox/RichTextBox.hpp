//
//  RichTextBox.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 19/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_RichTextBox_hpp
#define ddui_RichTextBox_hpp

#include <ddui/models/TextEdit>
#include <vector>

namespace RichTextBox {

struct RichTextBoxState {
    RichTextBoxState();

    TextEdit::Model* model;
    bool multiline = false;

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
    ddui::Color border_color = ddui::rgb(0xc8c8c8);
    ddui::Color border_color_focused = ddui::rgb(0x3264ff);
    ddui::Color bg_color = ddui::rgb(0xffffff);
    ddui::Color bg_color_focused = ddui::rgb(0xffffff);
    ddui::Color cursor_color = ddui::rgb(0x3264ff);
    ddui::Color selection_color = ddui::rgba(0x3264ff, 0.4);
    bool selection_in_foreground = true;
};

void update(RichTextBoxState* state);

}

#endif
