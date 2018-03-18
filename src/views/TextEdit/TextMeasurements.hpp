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
#include <vector>

namespace TextMeasurements {

struct Character {
    float x, y, width, height; // Bounding rect
    float max_x, baseline, ascender, line_height; // Character properties
};

struct LineMeasurements {
    float y;
    float line_height;
    float height;
    float max_ascender;

    std::vector<Character> characters;
};

struct Measurements {
    float width, height;
    std::vector<LineMeasurements> lines;
};

Measurements measure(Context ctx,
                     const TextEditModel::Model* model,
                     std::function<void(Context,int,int*,int*)> measure_entity);

LineMeasurements measure(Context ctx,
                         const TextEditModel::Model* model,
                         const TextEditModel::Line* line,
                         std::function<void(Context,int,int*,int*)> measure_entity);

void locate_selection_point(const Measurements* measurements, int x, int y, int* lineno, int* index);

}

#endif
