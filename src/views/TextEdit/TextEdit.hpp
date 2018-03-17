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

    TextEditModel* model;

    // Private data
    int current_version_count;
    std::vector<LineMeasurements> measurements;

    // UI info
    int content_width;
    int content_height;
    ScrollArea::ScrollAreaState scroll_area_state;
};

void update(TextEditState* state,
            Context ctx,
            std::function<void(Context,int,int*,int*)> measure_entity,
            std::function<void(Context,int)> update_entity);

}

#endif
