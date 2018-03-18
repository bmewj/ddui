//
//  app.linux.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 11/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "app.hpp"
#include "keyboard.hpp"
#include "views/TextEdit/TextEditCaret.hpp"

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
FocusState focus;
KeyState key_state = { 0 };
char key_character[7];
Cursor current_cursor = CURSOR_ARROW;
NSCursor* cursors[CURSOR_COUNT];

static void getGlVersion(int *major, int *minor);
static void getGlslVersion(int *major, int *minor);
static bool glInit();
static void init_cursors();

static void error_callback(int error, const char* desc);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void character_callback(GLFWwindow* window, unsigned int codepoint);
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
    glfwSetCharCallback(window, character_callback);
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

    focus.focus_old = NULL;
    focus.focus_new = NULL;
  
    app::load_font_face("entypo", "assets/Entypo.ttf");
    TextEditCaret::init();

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
    ctx.focus = &focus;
    ctx.key = &key_state;
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
    
    focus.action = FocusState::NO_CHANGE;
    focus.groups.clear();
    
    update_function(ctx);

    mouse.scroll_dx = 0;
    mouse.scroll_dy = 0;

    if (key_state.action == keyboard::ACTION_RELEASE) {
        if (key_state.key == keyboard::KEY_TAB) {
            if (key_state.mods & keyboard::MOD_SHIFT) {
                keyboard::tab_backward(ctx);
            } else {
                keyboard::tab_forward(ctx);
            }
            keyboard::consume_key_event(ctx);
        }
    }
    
    key_state.character = NULL;
    key_state.action = 0;
    key_state.key = 0;
    
    focus.focus_old = focus.focus_new;
    focus.focus_new = NULL;
    
    int group_index = -1;
    for (int i = 0; i < focus.groups.size(); ++i) {
        if (focus.groups[i] == focus.focus_old) {
            group_index = i;
            break;
        }
    }
    
    switch (focus.action) {
        case FocusState::NO_CHANGE:
            if (group_index != -1) {
                focus.focus_new = focus.focus_old;
            }
            break;
        case FocusState::TAB_FORWARD:
            if (group_index != -1 && group_index + 1 < focus.groups.size()) {
                focus.focus_new = focus.groups[group_index + 1];
            } else if (focus.focus_old == NULL && !focus.groups.empty()) {
                focus.focus_new = focus.groups.front();
            }
            break;
        case FocusState::TAB_BACKWARD:
            if (group_index != -1 && group_index - 1 >= 0) {
                focus.focus_new = focus.groups[group_index - 1];
            } else if (focus.focus_old == NULL && !focus.groups.empty()) {
                focus.focus_new = focus.groups.back();
            }
            break;
        case FocusState::TAB_TO:
            for (int i = 0; i < focus.groups.size(); ++i) {
                if (focus.groups[i] == focus.tab_to) {
                    focus.focus_new = focus.tab_to;
                    break;
                }
            }
            break;
        case FocusState::BLUR:
            break;
    }
    
    if (focus.focus_old != focus.focus_new) {
        must_repaint = true;
    }

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

const char* app::get_clipboard_string() {
    return glfwGetClipboardString(window);
}

void app::set_clipboard_string(const char* string) {
    glfwSetClipboardString(window, string);
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
    key_state.action = action;
    key_state.key = key;
    key_state.mods = mods;
}

void character_callback(GLFWwindow* window, unsigned int codepoint) {
    auto cp = codepoint;

    int n = 0;
    if (cp < 0x80) n = 1;
    else if (cp < 0x800) n = 2;
    else if (cp < 0x10000) n = 3;
    else if (cp < 0x200000) n = 4;
    else if (cp < 0x4000000) n = 5;
    else if (cp <= 0x7fffffff) n = 6;

    key_character[n] = '\0';

    switch (n) {
        case 6: key_character[5] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x4000000;
        case 5: key_character[4] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x200000;
        case 4: key_character[3] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x10000;
        case 3: key_character[2] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x800;
        case 2: key_character[1] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0xc0;
        case 1: key_character[0] = cp;
    }

    key_state.character = key_character;
    key_state.key = 0;
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
