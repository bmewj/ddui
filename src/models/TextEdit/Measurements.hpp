//
//  Measurements.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 19/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_TextEdit_Measurements_hpp
#define ddui_TextEdit_Measurements_hpp

#include "Model.hpp"
#include <ddui/core>
#include <functional>
#include <vector>

namespace TextEdit {

struct CharacterMeasurements {
    float x, y, width, height; // Bounding rect
    float max_x, baseline, ascender, line_height; // Character properties
};

struct LineMeasurements {
    float y;
    float line_height;
    float height;
    float max_ascender;

    std::vector<CharacterMeasurements> characters;
};

struct Measurements {
    float width, height;
    std::vector<LineMeasurements> lines;
};

using MeasureEntityFn = std::function<void(int line, int index, int entity_id, float* width, float* height)>;

Measurements measure(const Model* model, const MeasureEntityFn& measure_entity);

LineMeasurements measure(const Model* model, int lineno, const MeasureEntityFn& measure_entity);

void locate_selection_point(const Measurements* measurements, float x, float y, int* lineno, int* index);

}

#endif
