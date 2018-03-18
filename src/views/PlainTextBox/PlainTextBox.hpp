//
//  PlainTextBox.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 18/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_PlainTextBox_hpp
#define ddui_PlainTextBox_hpp

#include <ddui/Context>
#include <ddui/views/ScrollArea>
#include <string>
#include "../TextEdit/TextEditModel.hpp"
#include "../TextEdit/TextMeasurements.hpp"
#include <vector>

namespace PlainTextBox {

struct PlainTextBoxState {
    PlainTextBoxState();

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
    NVGcolor border_color = nvgRGB(200, 200, 200);
    NVGcolor border_color_focused = nvgRGB(50, 100, 255);
    NVGcolor bg_color = nvgRGB(255, 255, 255);
    NVGcolor bg_color_focused = nvgRGB(255, 255, 255);
    NVGcolor cursor_color = nvgRGB(50, 100, 255);
    NVGcolor selection_color = nvgRGBA(50, 100, 255, 100);
};

void update(PlainTextBoxState* state, Context ctx);

}

#endif
