//
//  TextEditCaret.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 18/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "TextEditCaret.hpp"
#include <thread>
#include <mutex>
#include <chrono>
#include <ddui/app>

namespace TextEditCaret {

static std::mutex phase_mutex;
static bool phase;
static std::chrono::high_resolution_clock::time_point last_update;

static void run_thread();

static constexpr double flicker_rate = 1.5;

void init() {
    phase = true;
    last_update = std::chrono::high_resolution_clock::now();

    new std::thread(run_thread);
}

void run_thread() {
    while (true) {
        bool updated_phase = false;
        auto current_time = std::chrono::high_resolution_clock::now();

        phase_mutex.lock();

        double time_elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(current_time - last_update).count();

        if (time_elapsed > 1.0 / flicker_rate) {
            // Enough time has elapsed. Change the phase.
            phase = !phase;
            updated_phase = true;
            last_update = current_time;
            time_elapsed = 0.0;
        }

        phase_mutex.unlock();

        if (updated_phase) {
            app::post_empty_event();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds((int)(1000 * (1.0 / flicker_rate - time_elapsed))));
    }
}

bool get_phase() {
    phase_mutex.lock();
    auto the_phase = phase;
    phase_mutex.unlock();
    return the_phase;
}

void reset_phase() {
    phase_mutex.lock();
    phase = true;
    last_update = std::chrono::high_resolution_clock::now();
    phase_mutex.unlock();
}

}
