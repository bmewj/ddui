//
//  ScrollArea.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 11/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_ScrollArea_hpp
#define ddui_ScrollArea_hpp

#include <ddui/ddui>
#include <functional>
#include "ScrollAreaState.hpp"

namespace ScrollArea {

void update(ScrollAreaState* state, float inner_width, float inner_height, std::function<void()> update_inner);
void scroll_into_view(ScrollAreaState* state, float x, float y, float width, float height);

}

#endif
