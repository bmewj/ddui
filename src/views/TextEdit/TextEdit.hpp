//
//  TextEdit.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 12/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_TextEdit_hpp
#define ddui_TextEdit_hpp

#include <ddui/Context>
#include <ddui/views/ScrollArea>
#include "TextEditModel.hpp"
#include "TextMeasurements.hpp"
#include <vector>

namespace TextEdit {

struct TextEditState {
    TextEditState();

    TextEditModel::Model* model;

    // Private data
    int current_version_count;
    TextMeasurements::Measurements measurements;

    // UI info
    bool is_mouse_dragging;
    int content_width;
    int content_height;
    ScrollArea::ScrollAreaState scroll_area_state;
};

void draw_content(Context ctx,
                  float offset_x, float offset_y,
                  const TextEditModel::Model* model,
                  const TextMeasurements::Measurements* measurements,
                  std::function<void(Context,int)> update_entity);

void draw_selection(Context ctx,
                    float offset_x, float offset_y,
                    const TextEditModel::Model* model,
                    const TextMeasurements::Measurements* measurements,
                    NVGcolor cursor_color,
                    NVGcolor selection_color);

void update(TextEditState* state,
            Context ctx,
            std::function<void(Context,int,int*,int*)> measure_entity,
            std::function<void(Context,int)> update_entity);

}

#endif
