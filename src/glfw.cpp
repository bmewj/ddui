//
//  glfw.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 26/08/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "glfw.hpp"
#include "core.hpp"
#include <functional>

#ifdef __APPLE__
#include <AppKit/AppKit.h>
#endif

namespace ddui {

static bool cursors_initialised = false;
static void init_cursors();
static void set_window_cursor(GLFWwindow* window, ddui::Cursor cursor);

static void get_gl_version(int* major, int* minor);
static void get_glsl_version(int* major, int* minor);
static bool init_gl();

static void error_callback(int error, const char* desc);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void character_callback(GLFWwindow* window, unsigned int codepoint);
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
static void window_size_callback(GLFWwindow* window, int width, int height);

bool init_glfw() {
    if (!glfwInit()) {
        printf("Failed to init GLFW.\n");
        return false;
    }
    
    glfwSetErrorCallback(error_callback);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 0);
#endif

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    return true;
}

bool init_window(GLFWwindow* window, std::function<void()> update_proc) {

    auto update_proc_ptr = new std::function<void()>(std::move(update_proc));
    glfwSetWindowUserPointer(window, update_proc_ptr);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCharCallback(window, character_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);
    
    glfwMakeContextCurrent(window);
    if (!init_gl()) {
        printf("Could not init GL3 API\n");
        glfwTerminate();
        return false;
    }

    ddui::set_post_empty_message_proc(glfwPostEmptyEvent);
    ddui::set_set_cursor_proc(std::bind(set_window_cursor, window, std::placeholders::_1));

    if (!cursors_initialised) {
        cursors_initialised = true;
        init_cursors();
    }

    glfwSwapInterval(0);
    glfwSetTime(0);

    return true;
}

void update_window(GLFWwindow* window) {
    auto update_proc_ptr = (std::function<void()>*)glfwGetWindowUserPointer(window);

    int fb_width, fb_height, win_width, win_height;
    glfwGetFramebufferSize(window, &fb_width, &fb_height);
    glfwGetWindowSize(window, &win_width, &win_height);
    auto pixel_ratio = (float)fb_width / (float)win_width;
    
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    ddui::input_mouse_position(xpos, ypos);
    
    ddui::update(win_width, win_height, pixel_ratio, *update_proc_ptr);
    glfwSwapBuffers(window);
}

void get_gl_version(int* major, int* minor) {
    const char *verstr = (const char *) glGetString(GL_VERSION);
    if ((verstr == NULL) || (sscanf(verstr,"%d.%d", major, minor) != 2)) {
        *major = *minor = 0;
        fprintf(stderr, "Invalid GL_VERSION format!!!\n");
    }
}

void get_glsl_version(int* major, int* minor) {
    int gl_major, gl_minor;
    get_gl_version(&gl_major, &gl_minor);

    *major = *minor = 0;
    if (gl_major == 1) {
        /* GL v1.x can only provide GLSL v1.00 as an extension */
        const char *extstr = (const char *) glGetString(GL_EXTENSIONS);
        if ((extstr != NULL) &&
                (strstr(extstr, "GL_ARB_shading_language_100") != NULL)) {
            *major = 1;
            *minor = 0;
        }
    } else if (gl_major >= 2) {
        /* GL v2.0 and greater must parse the version string */
        const char* verstr =
            (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

        if ((verstr == NULL) ||
                (sscanf(verstr, "%d.%d", major, minor) != 2)) {
            *major = *minor = 0;
            fprintf(stderr, "Invalid GL_SHADING_LANGUAGE_VERSION format!!!\n");
        }
    }
}

bool init_gl() {
    if (gl3wInit()) {
        printf("Problem initializing OpenGL\n");
        return false;
    }

    int maj, min, slmaj, slmin;
    get_gl_version(&maj, &min);
    get_glsl_version(&slmaj, &slmin);

    printf("OpenGL version: %d.%d\n", maj, min);
    printf("GLSL version: %d.%d\n", slmaj, slmin);

    return true;
}

void error_callback(int error, const char* desc) {
    printf("GLFW error. %s\n", desc);
    exit(0);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    ddui::input_key(key, scancode, action, mods);
}

void character_callback(GLFWwindow* window, unsigned int codepoint) {
    ddui::input_character(codepoint);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    ddui::input_mouse_button(button, action, mods);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
#ifdef __APPLE__
    ddui::input_scroll(-10.0 * xoffset, -10.0 * yoffset);
#else
    ddui::input_scroll(-20.0 * xoffset, -20.0 * yoffset);
#endif
}

void window_size_callback(GLFWwindow* window, int win_width, int win_height) {
    update_window(window);
}

#ifdef __APPLE__

static NSCursor* cursors[CURSOR_COUNT];
static NSCursor* current_cursor;

void init_cursors() {
    current_cursor = NULL;
    cursors[CURSOR_ARROW]             = [NSCursor arrowCursor];
    cursors[CURSOR_IBEAM]             = [NSCursor IBeamCursor];
    cursors[CURSOR_CROSS_HAIR]        = [NSCursor crosshairCursor];
    cursors[CURSOR_POINTING_HAND]     = [NSCursor pointingHandCursor];
    cursors[CURSOR_OPEN_HAND]         = [NSCursor openHandCursor];
    cursors[CURSOR_CLOSED_HAND]       = [NSCursor closedHandCursor];
    cursors[CURSOR_HORIZONTAL_RESIZE] = [NSCursor resizeLeftRightCursor];
    cursors[CURSOR_VERTICAL_RESIZE]   = [NSCursor resizeUpDownCursor];
}

void set_window_cursor(GLFWwindow* window, ddui::Cursor cursor) {
    if (current_cursor) {
        [current_cursor pop];
    }
    current_cursor = cursors[cursor];
    [current_cursor push];
}

#else

static GLFWcursor* cursors[ddui::CURSOR_COUNT];

void init_cursors() {
    cursors[ddui::CURSOR_ARROW]             = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    cursors[ddui::CURSOR_IBEAM]             = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    cursors[ddui::CURSOR_CROSS_HAIR]        = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    cursors[ddui::CURSOR_POINTING_HAND]     = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
    cursors[ddui::CURSOR_OPEN_HAND]         = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
    cursors[ddui::CURSOR_CLOSED_HAND]       = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
    cursors[ddui::CURSOR_HORIZONTAL_RESIZE] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
    cursors[ddui::CURSOR_VERTICAL_RESIZE]   = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
}

void set_window_cursor(GLFWwindow* window, ddui::Cursor cursor) {
    glfwSetCursor(window, cursors[cursor]);
}

#endif

}
