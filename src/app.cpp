//
//  app.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 30/08/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "app.hpp"
#include "core.hpp"
#include "glfw.hpp"

namespace ddui {

bool app_init(int window_width, int window_height, const char* title, std::function<void()> update_proc) {
	auto ddui_state = get_state();
    if (!ddui::init_glfw()) {
        printf("Failed to init GLFW.\n");
        return false;
    }

    ddui_state->glfw_window = glfwCreateWindow(window_width, window_height, title, NULL, NULL);
    if (!ddui_state->glfw_window) {
        glfwTerminate();
        return false;
    }

    ddui::init_window(ddui_state->glfw_window, update_proc);
    if (!ddui::init()) {
        printf("Could not init ddui.\n");
        return false;
    }

    return true;
}

void app_run() {
	auto ddui_state = get_state();

    while (!glfwWindowShouldClose(ddui_state->glfw_window)) {
        ddui::update_window(ddui_state->glfw_window);
        if (ddui::animation::is_animating()) {
            glfwWaitEventsTimeout(15e-3);
        } else {
            glfwWaitEvents();
        }
    }
}

}
