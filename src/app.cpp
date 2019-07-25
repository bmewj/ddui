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

static GLFWwindow* window;

bool app_init(int window_width, int window_height, const char* title, std::function<void()> update_proc) {
    if (!ddui::init_glfw()) {
        printf("Failed to init GLFW.\n");
        return false;
    }

    window = glfwCreateWindow(window_width, window_height, title, NULL, NULL);
    if (!window) {
        glfwTerminate();
        return false;
    }

    ddui::init_window(window, update_proc);
    if (!ddui::init()) {
        printf("Could not init ddui.\n");
        return false;
    }

    return true;
}

void app_run() {
    while (!glfwWindowShouldClose(window)) {
        ddui::update_window(window);
        if (ddui::animation::is_animating()) {
            glfwWaitEventsTimeout(15e-3);
        } else {
            glfwWaitEvents();
        }
    }
}

}
