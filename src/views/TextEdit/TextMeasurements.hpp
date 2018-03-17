//
//  TextMeasurements.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 13/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_TextMeasurements_hpp
#define ddui_TextMeasurements_hpp

#include "TextEditModel.hpp"
#include <ddui/Context>
#include <functional>

namespace TextEdit {

struct LineMeasurements {
    float y;
    float line_height;
    float height;
    float max_ascender;
    
    struct Character {
        float x, y;
        float max_x, line_height; // For characters
        float width, height; // For entities
    };

    std::vector<Character> characters;
};

LineMeasurements measure(Context ctx,
                         const TextEditModel::Line* line,
                         float min_line_height,
                         const char* regular_font,
                         const char* bold_font,
                         std::function<void(Context,int,int*,int*)> measure_entity);

void locate_selection_point(const std::vector<LineMeasurements>* measurements, int x, int y, int* lineno, int* index);

}

#endif
