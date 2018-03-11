//
//  ScrollArea.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 11/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_ScrollArea_hpp
#define ddui_ScrollArea_hpp

#include <ddui/Context>
#include <functional>

namespace ScrollArea {

struct ScrollAreaState {
    ScrollAreaState();

    int scroll_x;
    int scroll_y;
    int initial_scroll_x;
    int initial_scroll_y;
    bool is_dragging_horizontal_bar;
    bool is_dragging_vertical_bar;
};

void update(ScrollAreaState* state, Context ctx, int inner_width, int inner_height, std::function<void(Context)> update_inner);

}

#endif
