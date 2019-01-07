//
//  profiling.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 14/12/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "profiling.hpp"
#include "core.hpp"
#include <ddui/util/get_content_filename>
#include <mutex>
#include <string>
#include <string.h>
#include <sstream>
#include <iomanip>

namespace profiling {

static void write_to_buffer(std::string data);
static void add_profiling_entry(std::time_t start_time, bool is_animating, int num_set_immediates, int num_repaints, int duration);

int num_set_immediates = 0;
int num_repaints = 0;

static std::chrono::high_resolution_clock::time_point time_a, time_b;
static std::time_t start_time;
static std::mutex repaint_reasons_mutex;
static std::stringstream repaint_reasons;
static bool repaint_reasons_empty;

void frame_start() {
    time_a = std::chrono::high_resolution_clock::now();
    start_time = std::time(nullptr);
    num_set_immediates = 0;
    num_repaints = 0;
    repaint_reasons_mutex.lock();
    repaint_reasons = std::stringstream();
    repaint_reasons_empty = true;
    repaint_reasons_mutex.unlock();
}

void frame_end() {
    time_b = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(time_b - time_a).count();
    add_profiling_entry(start_time, ddui::animation::is_animating(), num_set_immediates, num_repaints, (int)(duration * 1000000.0));
}

void repaint_start() {
    repaint_reasons_mutex.lock();
    if (!repaint_reasons_empty) {
        repaint_reasons << '|';
        repaint_reasons_empty = true;
    }
    repaint_reasons_mutex.unlock();
}

void repaint_reason(const char* reason) {
    repaint_reasons_mutex.lock();
    if (num_repaints == 0) {
        repaint_reasons_mutex.unlock();
        return;
    }
    if (!repaint_reasons_empty) {
        repaint_reasons << '&';
    }
    repaint_reasons << reason;
    repaint_reasons_empty = false;
    repaint_reasons_mutex.unlock();
}

void add_profiling_entry(std::time_t start_time, bool is_animating, int num_set_immediates, int num_repaints, int duration) {
    std::stringstream ss; 
    ss << std::put_time(std::gmtime(&start_time), "%FT%T") << ',';
    ss << (is_animating ? '1' : '0') << ',';
    ss << num_set_immediates << ',';
    ss << num_repaints << ',';
    ss << duration << ',';
    repaint_reasons_mutex.lock();
    ss << repaint_reasons.str() << '\n';
    repaint_reasons_mutex.unlock();
    write_to_buffer(ss.str());
}

void write_to_buffer(std::string data) {

    static FILE* file_handle = NULL;
    
    // Open file
    if (file_handle == NULL) {
        auto filename = get_content_filename("profiling_data_2.csv");
        file_handle = fopen(filename.c_str(), "a");
    }

    // Write data
    fwrite(data.c_str(), 1, data.size(), file_handle);

    // Flush file
    static auto buffer_last_flushed = std::chrono::high_resolution_clock::now();
    auto current_time = std::chrono::high_resolution_clock::now();
    auto seconds_since_last_flush = (int)std::chrono::duration_cast<std::chrono::duration<double>>(
        current_time - buffer_last_flushed
    ).count();
    if (seconds_since_last_flush > 30) {
        fflush(file_handle);
    }

}

}
