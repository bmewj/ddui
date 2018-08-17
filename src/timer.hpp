//
//  timer.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 17/08/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_timer_hpp
#define ddui_timer_hpp

#include <functional>

namespace timer {

int set_timeout(std::function<void()> callback, long time_in_ms);
void clear_timeout(int timeout_id);

int set_interval(std::function<void()> callback, long time_in_ms);
void clear_interval(int interval_id);

void init(); // called internally by ddui

}

#endif
