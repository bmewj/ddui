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

    int scroll_x;
    int scroll_y;
    int initial_scroll_x;
    int initial_scroll_y;
    bool is_dragging_horizontal_bar;
    bool is_dragging_vertical_bar;
};

}

#endif