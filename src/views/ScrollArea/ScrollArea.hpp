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
#include "ScrollAreaState.hpp"

namespace ScrollArea {

void update(ScrollAreaState* state, Context ctx, int inner_width, int inner_height, std::function<void(Context)> update_inner);
void scroll_into_view(ScrollAreaState* state, Context ctx, int x, int y, int width, int height);

}

#endif
