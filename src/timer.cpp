//
//  timer.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 17/08/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "core.hpp"
#include <chrono>
#include <thread>
#include <vector>
#include <mutex>

static int schedule_timer(std::function<void()> callback, long duration_in_ms, bool repeat);
static void clear_timer(int timer_id);

int ddui::timer::set_timeout(std::function<void()> callback, long time_in_ms) {
    return schedule_timer(std::move(callback), time_in_ms, false);
}

void ddui::timer::clear_timeout(int timeout_id) {
    clear_timer(timeout_id);
}

int ddui::timer::set_interval(std::function<void()> callback, long time_in_ms) {
    return schedule_timer(std::move(callback), time_in_ms, true);
}

void ddui::timer::clear_interval(int interval_id) {
    clear_timer(interval_id);
}








struct Timer {
    int id;
    std::function<void()> callback;
    std::chrono::high_resolution_clock::duration duration;
    std::chrono::high_resolution_clock::time_point due_time;
    bool repeat;
    bool erase;
};

static void run_thread();

static std::mutex mutex;
static std::condition_variable condition_variable;
static std::vector<Timer> timers;
static int next_timer_id;
static std::chrono::high_resolution_clock::time_point thread_wake_time;

void timer_init() {
    next_timer_id = 1;
    std::thread(run_thread).detach();
}

void run_thread() {
    while (true) {

        std::unique_lock<std::mutex> lock(mutex);
        auto current_time = std::chrono::high_resolution_clock::now();

        // Call all callbacks that are due
        for (auto& timer : timers) {
            if (timer.due_time <= current_time) {
                if (timer.repeat) {
                    ddui::set_immediate(timer.callback);
                    timer.due_time = timer.due_time + timer.duration;
                } else {
                    ddui::set_immediate(std::move(timer.callback));
                    timer.erase = true;
                }
            }
        }

        // Erase timeouts
        for (int i = timers.size() - 1; i >= 0; --i) {
            if (timers[i].erase) {
                timers.erase(timers.begin() + i);
            }
        }

        // No timers left? Sleep
        if (timers.empty()) {
            condition_variable.wait(lock);
            continue;
        }

        // Determine next wake time
        thread_wake_time = timers.front().due_time;
        for (auto& timer : timers) {
            if (thread_wake_time > timer.due_time) {
                thread_wake_time = timer.due_time;
            }
        }

        // Convert to duration and sleep
        auto sleep_duration = thread_wake_time - current_time;
        condition_variable.wait_for(lock, sleep_duration);

    }
}

int schedule_timer(std::function<void()> callback, long duration_in_ms, bool repeat) {
    std::unique_lock<std::mutex> lock(mutex);

    bool thread_is_idle = timers.empty();

    Timer timer;
    timer.id = next_timer_id++;
    timer.callback = std::move(callback);
    timer.duration = std::chrono::milliseconds(duration_in_ms);
    timer.due_time = std::chrono::high_resolution_clock::now() + timer.duration;
    timer.repeat = repeat;
    timer.erase = false;
    timers.push_back(std::move(timer));

    if (thread_is_idle || timer.due_time < thread_wake_time) {
        condition_variable.notify_one();
    }
    
    return timers.back().id;
}

void clear_timer(int timer_id) {
    std::unique_lock<std::mutex> lock(mutex);

    for (int i = 0; i < timers.size(); ++i) {
        if (timers[i].id == timer_id) {
            timers.erase(timers.begin() + i);
            break;
        }
    }
}
