//
//  animation.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 13/07/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_animation_hpp
#define ddui_animation_hpp

#include "Context.hpp"

namespace animation {

void start(void* identifier);
void stop(void* identifier);
bool is_animating(void* identifier);
double get_time_elapsed(void* identifier);

void update_animation(); // called internally by ddui
bool is_animating();     // called internally by ddui

}

#endif
