//
//  Drawing.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 19/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_TextEdit_Drawing_hpp
#define ddui_TextEdit_Drawing_hpp

#include <ddui/Context>
#include <ddui/views/ScrollArea>
#include "Model.hpp"
#include "Measurements.hpp"
#include <vector>

namespace TextEdit {

void draw_content(Context ctx,
                  float offset_x, float offset_y,
                  const Model* model,
                  const Measurements* measurements,
                  std::function<void(Context,int)> update_entity);

void draw_selection(Context ctx,
                    float offset_x, float offset_y,
                    const Model* model,
                    const Measurements* measurements,
                    NVGcolor cursor_color,
                    NVGcolor selection_color);

}

#endif
