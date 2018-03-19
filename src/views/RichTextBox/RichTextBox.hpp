//
//  RichTextBox.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 19/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_RichTextBox_hpp
#define ddui_RichTextBox_hpp

#include <ddui/Context>
#include <ddui/views/ScrollArea>
#include <string>
#include "../TextEdit/TextEditModel.hpp"
#include "../TextEdit/TextMeasurements.hpp"
#include <vector>

namespace RichTextBox {

struct RichTextBoxState {
    RichTextBoxState();

    TextEditModel::Model* model;
    bool multiline = false;

    // Text edit state
    int current_version_count;
    TextMeasurements::Measurements measurements;

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

void update(RichTextBoxState* state, Context ctx);

}

#endif
