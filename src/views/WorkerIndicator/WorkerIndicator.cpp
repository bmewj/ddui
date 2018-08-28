//
//  WorkerIndicator.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 03/08/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "WorkerIndicator.hpp"
#include <cmath>

namespace WorkerIndicator {

using namespace ddui;

void update(Color color) {

    auto ANIMATION_ID = (void*)0x926492;

    auto cx = 0.5 * view.width;
    auto cy = 0.5 * view.height;
    
    constexpr auto pi = 3.1415;
    constexpr auto rs = 0.8 * pi; // ring size
    constexpr auto rw = 7.0;      // ring width
    
    constexpr auto r1 = 24.0;
    constexpr auto r2 = r1 - rw;
    
    if (!animation::is_animating(ANIMATION_ID)) {
        animation::start(ANIMATION_ID);
    }
    
    constexpr auto cycle_period = 0.5;
    auto elapsed = animation::get_time_elapsed(ANIMATION_ID);
    auto rotation = 2 * pi * (elapsed - cycle_period * (int)(elapsed / cycle_period)) / cycle_period;
    
    save();
    translate(cx, cy);
    rotate(rotation);

    begin_path();
    move_to(-r1, 0);
    arc(0, 0, r1, -pi, rs - pi, direction::CLOCKWISE);
    line_to(-r2 * cos(rs), -r2 * sin(rs));
    arc(0, 0, r2, rs - pi, -pi, direction::COUNTER_CLOCKWISE);
    close_path();

    fill_color(color);
    fill();
    
    restore();

}

}

