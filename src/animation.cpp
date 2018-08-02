//
//  animation.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 13/07/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "animation.hpp"
#include <chrono>

namespace animation {

struct ActiveAnimation {
    void* identifier;
    std::chrono::high_resolution_clock::time_point start_time;
    bool touched;
};

static std::chrono::high_resolution_clock::time_point last_update_time;
static std::vector<ActiveAnimation> active_animations;

static int find_active_animation(void* identifier);

void start(void* identifier) {

    ActiveAnimation new_animation;
    new_animation.identifier = identifier;
    new_animation.start_time = std::chrono::high_resolution_clock::now();
    new_animation.touched = true;

    int i = find_active_animation(identifier);
    if (i != -1) {
        active_animations[i] = new_animation;
    } else {
        active_animations.push_back(new_animation);
    }
}

void stop(void* identifier) {

    int i = find_active_animation(identifier);
    if (i != -1) {
        active_animations.erase(active_animations.begin() + i);
    }
}

bool is_animating(void* identifier) {

    int i = find_active_animation(identifier);
    if (i == -1) {
        return false;
    }

    active_animations[i].touched = true;
    return true;
}

double get_time_elapsed(void* identifier) {

    int i = find_active_animation(identifier);
    if (i == -1) {
        return 0.0;
    }

    auto start_time = active_animations[i].start_time;
    double time_elapsed = (
        std::chrono::duration_cast<std::chrono::duration<double>>
        (last_update_time - start_time).count()
    );

    active_animations[i].touched = true;
    return time_elapsed;
}

double ease_in(double completion) {
    return completion * completion;
}

double ease_out(double completion) {
    return 1 - (1 - completion) * (1 - completion);
}

double ease_in_out(double completion) {
    if (completion < 0.5) {
        return 0.5 * ease_in(2.0 * completion);
    } else {
        return 0.5 + 0.5 * ease_out(2.0 * completion - 1.0);
    }
}

void update_animation() {

    // Update the current time
    last_update_time = std::chrono::high_resolution_clock::now();

    // Remove inactive animations
    for (int i = active_animations.size() - 1; i >= 0; --i) {
        if (!active_animations[i].touched) {
            active_animations.erase(active_animations.begin() + i);
        }
    }

    // Reset touches
    for (auto& active_animation : active_animations) {
        active_animation.touched = false;
    }
}

bool is_animating() {
    return !active_animations.empty();
}

int find_active_animation(void* identifier) {
    for (int i = 0; i < active_animations.size(); ++i) {
        if (active_animations[i].identifier == identifier) {
            return i;
        }
    }
    return -1;
}

}
