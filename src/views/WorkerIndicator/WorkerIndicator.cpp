//
//  WorkerIndicator.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 03/08/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "WorkerIndicator.hpp"
#include <ddui/animation>
#include <cmath>

namespace WorkerIndicator {

void update(Context ctx, NVGcolor color) {

    auto ANIMATION_ID = (void*)0x926492;

    auto cx = 0.5 * ctx.width;
    auto cy = 0.5 * ctx.height;
    
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
    
    nvgSave(ctx.vg);
    nvgTranslate(ctx.vg, cx, cy);
    nvgRotate(ctx.vg, rotation);

    nvgBeginPath(ctx.vg);
    nvgMoveTo(ctx.vg, -r1, 0);
    nvgArc(ctx.vg, 0, 0, r1, -pi, rs - pi, NVG_CW);
    nvgLineTo(ctx.vg, -r2 * cos(rs), -r2 * sin(rs));
    nvgArc(ctx.vg, 0, 0, r2, rs - pi, -pi, NVG_CCW);
    nvgClosePath(ctx.vg);

    nvgFillColor(ctx.vg, color);
    nvgFill(ctx.vg);
    
    nvgRestore(ctx.vg);

}

}

