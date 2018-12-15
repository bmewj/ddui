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
#include <string>
#include <sstream>
#include <iomanip>

namespace profiling {

static void write_to_buffer(std::string data);
static void add_profiling_entry(std::time_t start_time, bool is_animating, int num_set_immediates, int num_repaints, int duration);

int num_set_immediates = 0;
int num_repaints = 0;

static std::chrono::high_resolution_clock::time_point time_a, time_b;
static std::time_t start_time;

void frame_start() {
    time_a = std::chrono::high_resolution_clock::now();
    start_time = std::time(nullptr);
    num_set_immediates = 0;
    num_repaints = 0;
}

void frame_end() {
    time_b = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(time_b - time_a).count();
    add_profiling_entry(start_time, ddui::animation::is_animating(), num_set_immediates, num_repaints, (int)(duration * 1000000.0));
}

void add_profiling_entry(std::time_t start_time, bool is_animating, int num_set_immediates, int num_repaints, int duration) {
    std::stringstream ss; 
    ss << std::put_time(std::gmtime(&start_time), "%FT%T") << ',';
    ss << (is_animating ? '1' : '0') << ',';
    ss << num_set_immediates << ',';
    ss << num_repaints << ',';
    ss << duration << '\n';
    write_to_buffer(ss.str());
}

void write_to_buffer(std::string data) {

    constexpr size_t BUFFER_SIZE = 12800;
    static char buffer[BUFFER_SIZE];
    static size_t buffer_end = 0;
    static FILE* file_handle = NULL;
    
    // Flush timer
    static auto buffer_last_flushed = std::chrono::high_resolution_clock::now();
    auto current_time = std::chrono::high_resolution_clock::now();
    auto seconds_since_last_flush = (int)std::chrono::duration_cast<std::chrono::duration<double>>(
        current_time - buffer_last_flushed
    ).count();

    // Flush buffer
    if (buffer_end + data.size() >= BUFFER_SIZE || seconds_since_last_flush > 30) {

        // Reset timer
        buffer_last_flushed = current_time;

        // Open file
        if (file_handle == NULL) {
            auto filename = get_content_filename("profiling_data.csv");
            file_handle = fopen(filename.c_str(), "a");
        }

        fwrite(buffer, 1, buffer_end, file_handle);
        fflush(file_handle);
        buffer_end = 0;
    }

    strcpy(&buffer[buffer_end], data.c_str());
    buffer_end += data.size();

}

}
