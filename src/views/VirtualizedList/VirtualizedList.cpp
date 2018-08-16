//
//  VirtualizedList.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 15/08/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "VirtualizedList.hpp"

namespace VirtualizedList {

void clear_measurements(State* state) {
    state->offsets.clear();
}

void update(State* state, Context ctx, int number_of_elements,
            std::function<float(int,Context)> measure_element_height,
            std::function<void(int,Context)> update_element,
            std::function<void(Context)> update_space_below) {

    // Number of elements changed, clear measurements
    if (state->offsets.size() != number_of_elements + 1) {
        state->offsets.clear();
    }

    // If the width has changed, clear measurments
    if (state->width != ctx.width) {
        state->offsets.clear();
        state->width = ctx.width;
    }

    // Remeasure all offsets
    if (state->offsets.empty()) {
        state->offsets.reserve(number_of_elements + 1);
        state->offsets.push_back(0.0);
        float accumulator = 0.0;
        for (auto i = 0; i < number_of_elements; ++i) {
            accumulator += measure_element_height(i, ctx);
            state->offsets.push_back(accumulator);
        }
    }

    auto outer_height = ctx.height;

    ScrollArea::update(&state->scroll_area, ctx, ctx.width, state->offsets.back(), [&](Context ctx) {

        // View port
        auto lower_y = state->scroll_area.scroll_y;
        auto upper_y = lower_y + outer_height;

        // Find lower offset
        auto lower = 0;
        for (; lower < number_of_elements; ++lower) {
            if (state->offsets[lower + 1] > lower_y) {
                break;
            }
        }

        // Find upper offset
        auto upper = lower;
        for (; upper < number_of_elements; ++upper) {
            if (state->offsets[upper] > upper_y) {
                break;
            }
        }

        // Draw stuff
        float offset_a;
        float offset_b = state->offsets[lower];

        for (auto i = lower; i < upper; ++i) {
            offset_a = offset_b;
            offset_b = state->offsets[i + 1];
            
            auto child_ctx = child_context(ctx, 0, offset_a, ctx.width, offset_b - offset_a);
            update_element(i, child_ctx);
            nvgRestore(ctx.vg);
        }

        // Update space below
        float space_after = outer_height - state->offsets.back();
        if (space_after > 0) {
            auto child_ctx = child_context(ctx, 0, state->offsets.back(), ctx.width, space_after);
            update_space_below(child_ctx);
            nvgRestore(ctx.vg);
        }

    });

}

}
