//
//  ScrollAreaState.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 14/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_ScrollAreaState_hpp
#define ddui_ScrollAreaState_hpp

namespace ScrollArea {

struct ScrollAreaState {
    ScrollAreaState();

    float scroll_x;
    float scroll_y;
    float initial_scroll_x;
    float initial_scroll_y;
    bool is_dragging_horizontal_bar;
    bool is_dragging_vertical_bar;

    struct {
        bool requested;
        float x, y, width, height;
    } scroll_into_view;
};

}

#endif
