//
//  app.linux.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 11/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "app.hpp"

#include <AppKit/AppKit.h>
#include <GL3/gl3w.h>

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>

#include <nanovg.h>
#define NANOVG_GL3_IMPLEMENTATION
#include <nanovg_gl.h>

GLFWwindow* window;
NSWindow* native_window;
NVGcontext* vg = NULL;
MouseState mouse = { 0 };
Cursor current_cursor = CURSOR_ARROW;
NSCursor* cursors[CURSOR_COUNT];

static void getGlVersion(int *major, int *minor);
static void getGlslVersion(int *major, int *minor);
static bool glInit();
static void init_cursors();

static void error_callback(int error, const char* desc);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

bool app::init(const char* title_bar) {
    printf("Running macOS\n");

    if (!glfwInit()) {
        printf("Failed to init GLFW.");
        return false;
    }

    glfwSetErrorCallback(error_callback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    
#ifdef DEMO_MSAA
    glfwWindowHint(GLFW_SAMPLES, 4);
#endif

    window = glfwCreateWindow(600, 400, title_bar, NULL, NULL);
    if (!window) {
        glfwTerminate();
        return false;
    }
  
    native_window = glfwGetCocoaWindow(window);
    if (!native_window) {
        glfwTerminate();
        return false;
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwMakeContextCurrent(window);
    if (!glInit()) {
        printf("Error: can't initialize GL3 API\n");
        glfwTerminate();
        return false;
    }

    init_cursors();
    
#ifdef DEMO_MSAA
    vg = nvgCreateGL3(NVG_STENCIL_STROKES | NVG_DEBUG);
#else
    vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
#endif
    if (vg == NULL) {
        printf("Could not init nanovg.\n");
        return false;
    }

    glfwSwapInterval(0);

    glfwSetTime(0);
  
    app::load_font_face("entypo", "assets/Entypo.ttf");

    return true;
}

void app::load_font_face(const char* name, const char* file_name) {
    nvgCreateFont(vg, name, file_name);
}

bool app::running() {
    return !glfwWindowShouldClose(window);
}

void app::update(std::function<void(Context)> update_function) {
    int fbWidth, fbHeight;
    int winWidth, winHeight;
    double mouseX, mouseY;
    float pxRatio;

    glfwGetWindowSize(window, &winWidth, &winHeight);
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glfwGetCursorPos(window, &mouseX, &mouseY);

    // Calculate pixel ration for hi-dpi devices.
    pxRatio = (float)fbWidth / (float)winWidth;

    // Update and render
    glViewport(0, 0, fbWidth, fbHeight);
    glClearColor(0.949f, 0.949f, 0.949f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

    nvgBeginFrame(vg, winWidth, winHeight, pxRatio);

    mouse.x = (int)mouseX;
    mouse.y = (int)mouseY;
    Cursor cursor = CURSOR_ARROW;
    bool must_repaint = false;

    Context ctx;
    ctx.vg = vg;
    ctx.mouse = &mouse;
    ctx.cursor = &cursor;
    ctx.must_repaint = &must_repaint;
    ctx.x = 0;
    ctx.y = 0;
    ctx.global_x = 0;
    ctx.global_y = 0;
    ctx.width = winWidth;
    ctx.height = winHeight;
    ctx.clip.x1 = 0;
    ctx.clip.y1 = 0;
    ctx.clip.x2 = winWidth;
    ctx.clip.y2 = winHeight;
    
    update_function(ctx);

    mouse.scroll_dx = 0;
    mouse.scroll_dy = 0;

    if (current_cursor != cursor) {
        [cursors[current_cursor] pop];
        current_cursor = cursor;
        [cursors[current_cursor] push];
    }

    nvgEndFrame(vg);

    glfwSwapBuffers(window);

    if (must_repaint) {
        glfwPostEmptyEvent();
    }
}

void app::wait_events() {
    glfwWaitEvents();
}

void app::terminate() {
    nvgDeleteGL3(vg);
    glfwTerminate();
}

void app::post_empty_event() {
    glfwPostEmptyEvent();
}

void getGlVersion(int *major, int *minor) {
    const char *verstr = (const char *) glGetString(GL_VERSION);
    if ((verstr == NULL) || (sscanf(verstr,"%d.%d", major, minor) != 2)) {
        *major = *minor = 0;
        fprintf(stderr, "Invalid GL_VERSION format!!!\n");
    }
}

void getGlslVersion(int *major, int *minor) {
    int gl_major, gl_minor;
    getGlVersion(&gl_major, &gl_minor);

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

bool glInit() {
    if (gl3wInit()) {
        printf("Problem initializing OpenGL\n");
        return false;
    }

    int maj, min, slmaj, slmin;
    getGlVersion(&maj, &min);
    getGlslVersion(&slmaj, &slmin);

    printf("OpenGL version: %d.%d\n", maj, min);
    printf("GLSL version: %d.%d\n", slmaj, slmin);

    return true;
}

void init_cursors() {
    cursors[CURSOR_ARROW]             = [NSCursor arrowCursor];
    cursors[CURSOR_IBEAM]             = [NSCursor IBeamCursor];
    cursors[CURSOR_CROSS_HAIR]        = [NSCursor crosshairCursor];
    cursors[CURSOR_POINTING_HAND]     = [NSCursor pointingHandCursor];
    cursors[CURSOR_OPEN_HAND]         = [NSCursor openHandCursor];
    cursors[CURSOR_CLOSED_HAND]       = [NSCursor closedHandCursor];
    cursors[CURSOR_HORIZONTAL_RESIZE] = [NSCursor resizeLeftRightCursor];
    cursors[CURSOR_VERTICAL_RESIZE]   = [NSCursor resizeUpDownCursor];
}

void error_callback(int error, const char* desc) {
    printf("GLFW error %d: %s\n", error, desc);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    NVG_NOTUSED(scancode);
    NVG_NOTUSED(mods);
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        mouse.accepted = false;
        mouse.pressed = false;
        mouse.pressed_secondary = false;
        mouse.initial_x = mouse.x;
        mouse.initial_y = mouse.y;

        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            mouse.pressed = true;
        } else {
            mouse.pressed_secondary = true;
        }
    }

    if (action == GLFW_RELEASE) {
        mouse.accepted = false;
        mouse.pressed = false;
        mouse.pressed_secondary = false;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    mouse.scroll_dx = - (int)(xoffset * 10.0);
    mouse.scroll_dy = - (int)(yoffset * 10.0);
}
