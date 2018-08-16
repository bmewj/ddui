//
//  VirtualizedList.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 15/08/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef VirtualizedList_hpp
#define VirtualizedList_hpp

#include <ddui/Context>
#include <ddui/views/ScrollArea>
#include <vector>

namespace VirtualizedList {

struct State {
    std::vector<float> offsets;
    float width;
    ScrollArea::ScrollAreaState scroll_area;
};

void clear_measurements(State* state);
void update(State* state, Context ctx, int number_of_elements,
            std::function<float(int,Context)> measure_element_height,
            std::function<void(int,Context)> update_element,
            std::function<void(Context)> update_space_below);

}

#endif
