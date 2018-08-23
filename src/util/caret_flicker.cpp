//
//  caret_flicker.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 19/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "caret_flicker.hpp"
#include <ddui/ddui>

namespace caret_flicker {

using namespace ddui;

static bool phase;
static int interval_id = -1;

static constexpr auto FLICKER_RATE = 1.5;
static constexpr auto FLICKER_TIME = (long)(1000.0 / FLICKER_RATE);

bool get_phase() {
    if (interval_id == -1) {
        reset_phase();
    }

    return phase;
}

void reset_phase() {
    timer::clear_interval(interval_id);
    phase = true;
    interval_id = timer::set_interval([]() {
        phase = !phase;
    }, FLICKER_TIME);
}

}
