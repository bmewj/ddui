//
//  ddui.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 23/08/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "ddui.hpp"
#include <vector>
#include <chrono>
#include <nanovg.h>

namespace ddui {

struct FocusState {
    void* focus_old;
    void* focus_new;
    std::vector<void*> groups;

    enum Action {
        NO_CHANGE,
        TAB_FORWARD,
        TAB_BACKWARD,
        TAB_TO,
        BLUR
    };

    Action action;
    void* tab_to;
};

// Globals
static NVGcontext* vg;
static FocusState focus_state;
static std::vector<KeyState> key_state_queue;
MouseState mouse_state;
KeyState key;
Cursor cursor;

// Setup
bool init() {

}

static void (*post_empty_message_proc)() = NULL;
void set_post_empty_message_proc(void (*proc)()) {
    post_empty_message_proc = proc;
}

static const char* (*get_clipboard_string_proc)() = NULL;
void set_get_clipboard_string_proc(const char* (*proc)()) {
    get_clipboard_string_proc = proc;
}

static void (*set_clipboard_string_proc)(const char*) = NULL;
void set_set_clipboard_string_proc(void (*proc)(const char*)) {
    set_clipboard_string_proc = proc;
}

static void (*set_cursor_proc)(Cursor) = NULL;
void set_set_cursor_proc(void (*proc)(Cursor)) {
    set_cursor_proc = proc;
}

// Teardown
void terminate() {

}

// User input
void input_key(int key, int scancode, int action, int mods) {
    
}

void input_character(unsigned int codepoint) {
    
}

void input_mouse_position(float x, float y) {
    mouse_state.x = x;
    mouse_state.y = y;
}

void input_mouse_button(int button, int action, int mods) {

}

void input_scroll(float offset_x, float offset_y) {
    
}

// Frame management
void begin_frame(float width, float height, float pixel_ratio) {

}

void end_frame() {

}

// Color utils
Color rgb(unsigned char r, unsigned char g, unsigned char b) {
    return Color {
        .r = r / (float)0xff,
        .g = g / (float)0xff,
        .b = b / (float)0xff,
        .a = 1.0
    };
}
Color rgb(unsigned int rgb) {
    return Color {
        .r = ((rgb >> 16) & 0xff) / (float)0xff,
        .g = ((rgb >>  8) & 0xff) / (float)0xff,
        .b = ((rgb >>  0) & 0xff) / (float)0xff,
        .a = 1.0
    };
}
Color rgba(unsigned char r, unsigned char g, unsigned char b, float a) {
    return Color {
        .r = r / (float)0xff,
        .g = g / (float)0xff,
        .b = b / (float)0xff,
        .a = a
    };
}
Color rgba(unsigned int rgb, float a) {
    return Color {
        .r = ((rgb >> 16) & 0xff) / (float)0xff,
        .g = ((rgb >>  8) & 0xff) / (float)0xff,
        .b = ((rgb >>  0) & 0xff) / (float)0xff,
        .a = a
    };
}

// State Handling
void save() {
    nvgSave(vg);
}

void restore() {
    nvgRestore(vg);
}

void reset() {
    nvgReset(vg);
}

// Render styles

// void shape_anti_alias(bool enabled);

void stroke_color(Color color) {
    nvgStrokeColor(vg, NVGcolor {
        .r = color.r,
        .g = color.g,
        .b = color.b,
        .a = color.a
    });
}

// void stroke_paint(NVGpaint paint);

void fill_color(Color color) {
    nvgFillColor(vg, NVGcolor {
        .r = color.r,
        .g = color.g,
        .b = color.b,
        .a = color.a
    });
}

// void fill_paint(NVGpaint paint);

// void miter_limit(float limit);

void stroke_width(float size) {
    nvgStrokeWidth(vg, size);
}

// void line_cap(int cap);

// void line_join(int join);

void global_alpha(float alpha) {
    nvgGlobalAlpha(vg, alpha);
}

// Transforms
void reset_transform() {
    nvgResetTransform(vg);
}

// void transform(float a, float b, float c, float d, float e, float f);

void translate(float x, float y) {
    nvgTranslate(vg, x, y);
}

void rotate(float angle) {
    nvgRotate(vg, angle);
}

// void skew_x(float angle);

// void skew_y(float angle);

void scale(float x, float y) {
    nvgScale(vg, x, y);
}

// void current_transform(float* xform);

// ...

// Images
// ...

// Paints
// ...

// Scissoring
void scissor(float x, float y, float w, float h) {
    nvgScissor(vg, x, y, w, h);
}

void reset_scissor() {
    nvgResetScissor(vg);
}

// Paths
void begin_path() {
    nvgBeginPath(vg);
}

void move_to(float x, float y) {
    nvgMoveTo(vg, x, y);
}

void line_to(float x, float y) {
    nvgLineTo(vg, x, y);
}

// void bezier_to(float c1x, float c1y, float c2x, float c2y, float x, float y);

// void quad_to(float cx, float cy, float x, float y);

// void arc_to(float x1, float y1, float x2, float y2, float radius);

void close_path() {
    nvgClosePath(vg);
}

// void path_winding(int dir);

void arc(float cx, float cy, float r, float a0, float a1, int dir) {
    nvgArc(vg, cx, cy, r, a0, a1, dir);
}

void rect(float x, float y, float w, float h) {
    nvgRect(vg, x, y, w, h);
}

void rounded_rect(float x, float y, float w, float h, float r) {
    nvgRoundedRect(vg, x, y, w, h, r);
}

void rounded_rect_varying(float x, float y, float w, float h, float radTopLeft, float radTopRight, float radBottomRight, float radBottomLeft) {
    nvgRoundedRectVarying(vg, x, y, w, h, radTopLeft, radTopRight, radBottomRight, radBottomLeft);
}

// void ellipse(float cx, float cy, float rx, float ry);

void circle(float cx, float cy, float r) {
    nvgCircle(vg, cx, cy, r);
}

void fill() {
    nvgFill(vg);
}

void stroke() {
    nvgStroke(vg);
}

// Text
int create_font(const char* name, const char* filename) {
    return nvgCreateFont(vg, name, filename);
}

void font_size(float size) {
    nvgFontSize(vg, size);
}

// void font_blur(float blur);

// void text_letter_spacing(float spacing);

// void text_line_height(float lineHeight);

void text_align(int align) {
    nvgTextAlign(vg, align);
}

void font_face(const char* font) {
    nvgFontFace(vg, font);
}

float text(float x, float y, const char* string, const char* end) {
    return nvgText(vg, x, y, string, end);
}

void text_box(float x, float y, float breakRowWidth, const char* string, const char* end) {
    nvgTextBox(vg, x, y, breakRowWidth, string, end);
}

float text_bounds(float x, float y, const char* string, const char* end, float* bounds) {
    return nvgTextBounds(vg, x, y, string, end, bounds);
}

void text_box_bounds(float x, float y, float breakRowWidth, const char* string, const char* end, float* bounds) {
    nvgTextBoxBounds(vg, x, y, breakRowWidth, string, end, bounds);
}

int text_glyph_positions(float x, float y, const char* string, const char* end, GlyphPosition* positions, int maxPositions) {
    return nvgTextGlyphPositions(vg, x, y, string, end, (NVGglyphPosition*)positions, maxPositions);
}

void text_metrics(float* ascender, float* descender, float* lineh) {
    nvgTextMetrics(vg, ascender, descender, lineh);
}

// int text_break_lines(const char* string, const char* end, float breakRowWidth, NVGtextRow* rows, int maxRows);

// Mouse state
bool mouse_hit(float x, float y, float width, float height) {
    return false;
}

bool mouse_hit_secondary(float x, float y, float width, float height) {
    return false;
}

bool mouse_over(float x, float y, float width, float height) {
    return false;
}

// Focus state
void register_focus_group(void* identifier) {
    focus_state.groups.push_back(identifier);
}

bool did_focus(void* identifier) {
    return (focus_state.focus_old != identifier &&
            focus_state.focus_new == identifier);
}

bool did_blur(void* identifier) {
    return (focus_state.focus_old == identifier &&
            focus_state.focus_new != identifier);
}

bool has_focus(void* identifier) {
    return (focus_state.focus_new == identifier);
}

void tab_forward() {
    focus_state.action = FocusState::TAB_FORWARD;
}

void tab_backward() {
    focus_state.action = FocusState::TAB_BACKWARD;
}

void focus(void* identifier) {
    focus_state.action = FocusState::TAB_TO;
    focus_state.tab_to = identifier;
}

void blur() {
    focus_state.action = FocusState::BLUR;
}

// Keyboard state
bool has_key_event() {
    return (key.character != NULL || key.key > 0);
}

bool has_key_event(void* identifier) {
    return (focus_state.focus_new == identifier) && (key.character != NULL || key.key > 0);
}

void consume_key_event() {
    key = { 0 };
}

void repeat_key_event() {
    key_state_queue.insert(key_state_queue.begin(), key);
    key = { 0 };
}

// Animation
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

// Timers
namespace timer {
    int set_timeout(std::function<void()> callback, long time_in_ms);
    int set_interval(std::function<void()> callback, long time_in_ms);
    void clear_timeout(int timeout_id);
    void clear_interval(int interval_id);
}

}
