//
//  core.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 23/08/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "core.hpp"
#include "init.hpp"
#include "timer.hpp"
#include "animation.hpp"
#include <vector>
#include <chrono>
#include <mutex>
#include <GL3/gl3w.h>

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
static std::mutex repaint_mutex;
static bool is_painting, should_repaint;
static std::mutex set_immediate_mutex;
static std::vector<std::function<void()>> set_immediate_callbacks;
static Cursor cursor_state_old, cursor_state_new;
MouseState mouse_state;
KeyState key_state;
Viewport view;
static std::vector<Viewport> saved_views;

// Setup
bool init() {
    vg = nvgCreate();
    if (vg == NULL) {
        printf("Could not init nanovg.\n");
        return false;
    }

    focus_state.focus_old = NULL;
    focus_state.focus_new = NULL;

    mouse_state = { 0 };
    key_state = { 0 };
    cursor_state_old = CURSOR_ARROW;
    cursor_state_new = CURSOR_ARROW;
  
    create_font("entypo", "assets/Entypo.ttf");
    timer_init();

    return true;
}


static std::function<void()> post_empty_message_proc;
void set_post_empty_message_proc(std::function<void()> proc) {
    post_empty_message_proc = std::move(proc);
}

static std::function<const char*()> get_clipboard_string_proc;
void set_get_clipboard_string_proc(std::function<const char*()> proc) {
    get_clipboard_string_proc = std::move(proc);
}

static std::function<void(const char*)> set_clipboard_string_proc;
void set_set_clipboard_string_proc(std::function<void(const char*)> proc) {
    set_clipboard_string_proc = std::move(proc);
}

static std::function<void(Cursor)> set_cursor_proc;
void set_set_cursor_proc(std::function<void(Cursor)> proc) {
    set_cursor_proc = std::move(proc);
}

// Teardown
void terminate() {
    nvgDelete(vg);
}

// User input
void input_key(int key, int scancode, int action, int mods) {
    KeyState key_state;
    key_state.action = action;
    key_state.key = key;
    key_state.mods = mods;
    key_state.character = NULL;
    key_state_queue.push_back(key_state);
}

void input_character(unsigned int codepoint) {
    auto cp = codepoint;

    int n = 0;
    if (cp < 0x80) n = 1;
    else if (cp < 0x800) n = 2;
    else if (cp < 0x10000) n = 3;
    else if (cp < 0x200000) n = 4;
    else if (cp < 0x4000000) n = 5;
    else if (cp <= 0x7fffffff) n = 6;

    auto key_character = new char[7];
    key_character[n] = '\0';

    switch (n) {
        case 6: key_character[5] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x4000000;
        case 5: key_character[4] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x200000;
        case 4: key_character[3] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x10000;
        case 3: key_character[2] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0x800;
        case 2: key_character[1] = 0x80 | (cp & 0x3f); cp = cp >> 6; cp |= 0xc0;
        case 1: key_character[0] = cp;
    }

    if (key_state_queue.empty()) {
        KeyState key_state;
        key_state.action = keyboard::ACTION_PRESS;
        key_state.key = 0;
        key_state.mods = 0;
        key_state.character = key_character;
        key_state_queue.push_back(key_state);
    } else {
        key_state_queue.back().key = 0;
        key_state_queue.back().character = key_character;
    }
}

void input_mouse_position(float x, float y) {
    mouse_state.x = x;
    mouse_state.y = y;
}

void input_mouse_button(int button, int action, int mods) {
    constexpr int ACTION_RELEASE = 0;
    constexpr int ACTION_PRESS = 1;
    constexpr int MOUSE_BUTTON_LEFT = 0;

    if (action == ACTION_PRESS) {
        mouse_state.accepted = false;
        mouse_state.pressed = false;
        mouse_state.pressed_secondary = false;
        mouse_state.initial_x = mouse_state.x;
        mouse_state.initial_y = mouse_state.y;

        if (button == MOUSE_BUTTON_LEFT) {
            mouse_state.pressed = true;
        } else {
            mouse_state.pressed_secondary = true;
        }
    }

    if (action == ACTION_RELEASE) {
        mouse_state.accepted = false;
        mouse_state.pressed = false;
        mouse_state.pressed_secondary = false;
    }
}

void input_scroll(float offset_x, float offset_y) {
    mouse_state.scroll_dx = offset_x;
    mouse_state.scroll_dy = offset_y;
}

// Frame management
static void update_pre(float width, float height, float pixel_ratio);
static void update_post();

void update(float width, float height, float pixel_ratio, std::function<void()> update_proc) {

    // Setup GL frame
    auto frame_buffer_width  = (int)(width * pixel_ratio);
    auto frame_buffer_height = (int)(height * pixel_ratio);
    glViewport(0, 0, frame_buffer_width, frame_buffer_height);
    glClearColor(0.949f, 0.949f, 0.949f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    repaint_mutex.lock();
    is_painting = true;
    should_repaint = false;
    repaint_mutex.unlock();

    while (true) {
        nvgBeginFrame(vg, width, height, pixel_ratio);
        update_pre(width, height, pixel_ratio);
        update_proc();
        update_post();

        repaint_mutex.lock();
        if (!should_repaint) {
            is_painting = false;
            repaint_mutex.unlock();
            break;
        }
        should_repaint = false;
        repaint_mutex.unlock();

        nvgCancelFrame(vg);
    }

    nvgEndFrame(vg);

}

void update_pre(float width, float height, float pixel_ratio) {

    // Process all set_immediate callbacks
    set_immediate_mutex.lock();
    auto callbacks = std::move(set_immediate_callbacks);
    set_immediate_mutex.unlock();
    for (auto& callback : callbacks) {
        callback();
    }

    // Let the animation system know that a new frame is being generated
    update_animation();

    cursor_state_new = CURSOR_ARROW;

    if (!key_state_queue.empty()) {
        key_state = key_state_queue.front();
        key_state_queue.erase(key_state_queue.begin());
    } else {
        key_state.action = 0;
        if (key_state.character) {
            delete[] key_state.character;
            key_state.character = NULL;    
        }
        key_state.key = 0;
    }

    focus_state.action = FocusState::NO_CHANGE;
    focus_state.groups.clear();

    view.width = width;
    view.height = height;
    view.clip.x1 = 0.0;
    view.clip.y1 = 0.0;
    view.clip.x2 = width;
    view.clip.y2 = height;
    saved_views.clear();
    saved_views.push_back(view);
}

void update_post() {

    mouse_state.scroll_dx = 0.0;
    mouse_state.scroll_dy = 0.0;

    if (key_state.action == keyboard::ACTION_RELEASE) {
        if (key_state.key == keyboard::KEY_TAB) {
            if (key_state.mods & keyboard::MOD_SHIFT) {
                tab_backward();
            } else {
                tab_forward();
            }
            consume_key_event();
        }
    }
    
    focus_state.focus_old = focus_state.focus_new;
    focus_state.focus_new = NULL;
    
    int group_index = -1;
    for (int i = 0; i < focus_state.groups.size(); ++i) {
        if (focus_state.groups[i] == focus_state.focus_old) {
            group_index = i;
            break;
        }
    }
    
    switch (focus_state.action) {
        case FocusState::NO_CHANGE:
            if (group_index != -1) {
                focus_state.focus_new = focus_state.focus_old;
            }
            break;
        case FocusState::TAB_FORWARD:
            if (group_index != -1 && group_index + 1 < focus_state.groups.size()) {
                focus_state.focus_new = focus_state.groups[group_index + 1];
            } else if (focus_state.focus_old == NULL && !focus_state.groups.empty()) {
                focus_state.focus_new = focus_state.groups.front();
            }
            break;
        case FocusState::TAB_BACKWARD:
            if (group_index != -1 && group_index - 1 >= 0) {
                focus_state.focus_new = focus_state.groups[group_index - 1];
            } else if (focus_state.focus_old == NULL && !focus_state.groups.empty()) {
                focus_state.focus_new = focus_state.groups.back();
            }
            break;
        case FocusState::TAB_TO:
            for (int i = 0; i < focus_state.groups.size(); ++i) {
                if (focus_state.groups[i] == focus_state.tab_to) {
                    focus_state.focus_new = focus_state.tab_to;
                    break;
                }
            }
            break;
        case FocusState::BLUR:
            break;
    }
    
    if (focus_state.focus_old != focus_state.focus_new ||
        !key_state_queue.empty()) {
        repaint();
    }

    if (cursor_state_old != cursor_state_new) {
        cursor_state_old  = cursor_state_new;
        if (set_cursor_proc) {
            set_cursor_proc(cursor_state_new);
        }
    }
}

void repaint() {
    repaint_mutex.lock();
    if (is_painting) {
        should_repaint = true;
    } else {
        if (post_empty_message_proc) {
            post_empty_message_proc();
        } else {
            printf("post_empty_message_proc not set! This is a crucial proc for ddui.\n");
        }
    }
    repaint_mutex.unlock();
}

void set_immediate(std::function<void()> callback) {
    set_immediate_mutex.lock();
    set_immediate_callbacks.push_back(std::move(callback));
    set_immediate_mutex.unlock();
    repaint();
}

// Color utils
Color rgb(unsigned char r, unsigned char g, unsigned char b) {
    Color color;
    color.r = r / (float)0xff;
    color.g = g / (float)0xff;
    color.b = b / (float)0xff;
    color.a = 1.0;
    return color;
}
Color rgb(unsigned int rgb) {
    Color color;
    color.r = ((rgb >> 16) & 0xff) / (float)0xff;
    color.g = ((rgb >>  8) & 0xff) / (float)0xff;
    color.b = ((rgb >>  0) & 0xff) / (float)0xff;
    color.a = 1.0;
    return color;
}
Color rgba(unsigned char r, unsigned char g, unsigned char b, float a) {
    Color color;
    color.r = r / (float)0xff;
    color.g = g / (float)0xff;
    color.b = b / (float)0xff;
    color.a = a;
    return color;
}
Color rgba(unsigned int rgb, float a) {
    Color color;
    color.r = ((rgb >> 16) & 0xff) / (float)0xff;
    color.g = ((rgb >>  8) & 0xff) / (float)0xff;
    color.b = ((rgb >>  0) & 0xff) / (float)0xff;
    color.a = a;
    return color;
}

// State Handling
void save() {
    nvgSave(vg);
    saved_views.push_back(view);
}

void restore() {
    nvgRestore(vg);
    view = saved_views.back();
    saved_views.pop_back();
}

void reset() {
    nvgReset(vg);
    view = saved_views.front();
    saved_views.clear();
    saved_views.push_back(view);
}

void sub_view(float x, float y, float width, float height) {
    save();
    nvgTranslate(vg, x, y);
    view.width = width;
    view.height = height;
}

// Render styles

void shape_anti_alias(bool enabled) {
    nvgShapeAntiAlias(vg, enabled);
}

void stroke_color(Color color) {
    nvgStrokeColor(vg, *(NVGcolor*)&color);
}

void stroke_paint(Paint paint) {
    nvgStrokePaint(vg, *(NVGpaint*)&paint);
}

void fill_color(Color color) {
    nvgFillColor(vg, *(NVGcolor*)&color);
}

void fill_paint(Paint paint) {
    nvgFillPaint(vg, *(NVGpaint*)&paint);
}

void miter_limit(float limit) {
    nvgMiterLimit(vg, limit);
}

void stroke_width(float size) {
    nvgStrokeWidth(vg, size);
}

void line_cap(int cap) {
    nvgLineCap(vg, cap);
}

void line_join(int join) {
    nvgLineJoin(vg, join);
}

void global_alpha(float alpha) {
    nvgGlobalAlpha(vg, alpha);
}

// Transforms
void reset_transform() {
    nvgResetTransform(vg);
}

void transform(float a, float b, float c, float d, float e, float f) {
    nvgTransform(vg, a, b, c, d, e, f);
}

void translate(float x, float y) {
    nvgTranslate(vg, x, y);
}

void rotate(float angle) {
    nvgRotate(vg, angle);
}

void skew_x(float angle) {
    nvgSkewX(vg, angle);
}

void skew_y(float angle) {
    nvgSkewY(vg, angle);
}

void scale(float x, float y) {
    nvgScale(vg, x, y);
    view.width  = view.width  / x;
    view.height = view.height / y;
}

void to_global_position(float* gx, float* gy, float x, float y) {
    float mat[6];
    nvgCurrentTransform(vg, mat);
    nvgTransformPoint(gx, gy, mat, x, y);
}

void from_global_position(float* x, float* y, float gx, float gy) {
    float mat[6], inv_mat[6];
    nvgCurrentTransform(vg, mat);
    nvgTransformInverse(inv_mat, mat);
    nvgTransformPoint(x, y, inv_mat, gx, gy);
}

void current_transform(float* xform) {
    nvgCurrentTransform(vg, xform);
}

// ...

// Images
// ...

// Paints
Paint linear_gradient(float sx, float sy, float ex, float ey, Color icol, Color ocol) {
    auto nvg_icol = *(NVGcolor*)&icol;
    auto nvg_ocol = *(NVGcolor*)&ocol;
    auto nvg_paint = nvgLinearGradient(vg, sx, sy, ex, ey, nvg_icol, nvg_ocol);
    return *(Paint*)&nvg_paint;
}

Paint box_gradient(float x, float y, float w, float h, float r, float f, Color icol, Color ocol) {
    auto nvg_icol = *(NVGcolor*)&icol;
    auto nvg_ocol = *(NVGcolor*)&ocol;
    auto nvg_paint = nvgBoxGradient(vg, x, y, w, h, r, f, nvg_icol, nvg_ocol);
    return *(Paint*)&nvg_paint;
}

Paint radial_gradient(float cx, float cy, float inr, float outr, Color icol, Color ocol) {
    auto nvg_icol = *(NVGcolor*)&icol;
    auto nvg_ocol = *(NVGcolor*)&ocol;
    auto nvg_paint = nvgRadialGradient(vg, cx, cy, inr, outr, nvg_icol, nvg_ocol);
    return *(Paint*)&nvg_paint;
}

Paint image_pattern(float ox, float oy, float ex, float ey, float angle, int image, float alpha) {
    auto nvg_paint = nvgImagePattern(vg, ox, oy, ex, ey, angle, image, alpha);
    return *(Paint*)&nvg_paint;
}

// Clipping
void clip(float x, float y, float width, float height) {
    nvgIntersectScissor(vg, x, y, width, height);
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

void bezier_to(float c1x, float c1y, float c2x, float c2y, float x, float y) {
    nvgBezierTo(vg, c1x, c1y, c2x, c2y, x, y);
}

void quad_to(float cx, float cy, float x, float y) {
    nvgQuadTo(vg, cx, cy, x, y);
}

void arc_to(float x1, float y1, float x2, float y2, float radius) {
    nvgArcTo(vg, x1, y1, x2, y2, radius);
}

void close_path() {
    nvgClosePath(vg);
}

void path_winding(int dir) {
    nvgPathWinding(vg, dir);
}

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

void ellipse(float cx, float cy, float rx, float ry) {
    nvgEllipse(vg, cx, cy, rx, ry);
}

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

void font_blur(float blur) {
    nvgFontBlur(vg, blur);
}

void text_letter_spacing(float spacing) {
    nvgTextLetterSpacing(vg, spacing);
}

void text_line_height(float lineHeight) {
    nvgTextLineHeight(vg, lineHeight);
}

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
#define NVG_MAX_STATES 32

struct NVGstate {
    NVGcompositeOperationState compositeOperation;
    int shapeAntiAlias;
    NVGpaint fill;
    NVGpaint stroke;
    float strokeWidth;
    float miterLimit;
    int lineJoin;
    int lineCap;
    float alpha;
    float xform[6];
    NVGscissor scissor;
    float fontSize;
    float letterSpacing;
    float lineHeight;
    float fontBlur;
    int textAlign;
    int fontId;
};

struct NVGcontext_ {
    NVGparams params;
    float* commands;
    int ccommands;
    int ncommands;
    float commandx, commandy;
    NVGstate states[NVG_MAX_STATES];
    int nstates;
};

static NVGscissor* get_scissor() {
    auto vg_ = (NVGcontext_*)vg;
    return &vg_->states[vg_->nstates - 1].scissor;
}

static bool is_point_inside_rect(float* xform, float* extent, float x, float y) {
    float inv_xform[6];
    nvgTransformInverse(inv_xform, xform);

    float x2, y2;
    nvgTransformPoint(&x2, &y2, inv_xform, x, y);

    return (
        (-extent[0] <= x2 && x2 < extent[0]) &&
        (-extent[1] <= y2 && y2 < extent[1])
    );
}

static bool mouse_inside(float x, float y, float width, float height) {
    float xform[6], extent[2];
    nvgCurrentTransform(vg, xform);
    
    extent[0] = width / 2;
    extent[1] = height / 2;
    
    nvgTransformPoint(&x, &y, xform, x + extent[0], y + extent[1]);
    xform[4] = x;
    xform[5] = y;
    
    if (!is_point_inside_rect(xform, extent, mouse_state.x, mouse_state.y)) {
        return false;
    }
    
    auto scissor = get_scissor();
    if (scissor->extent[0] < 0) {
        return true;
    }

    return is_point_inside_rect(scissor->xform, scissor->extent, mouse_state.x, mouse_state.y);
}

bool mouse_hit(float x, float y, float width, float height) {
    return (
        !mouse_state.accepted && mouse_state.pressed &&
        mouse_inside(x, y, width, height)
    );
}

bool mouse_hit_secondary(float x, float y, float width, float height) {
    return (
        !mouse_state.accepted && mouse_state.pressed_secondary &&
        mouse_inside(x, y, width, height)
    );
}

bool mouse_over(float x, float y, float width, float height) {
    return (
        !mouse_state.accepted && !mouse_state.pressed &&
        mouse_inside(x, y, width, height)
    );
}

void mouse_hit_accept() {
    mouse_state.accepted = true;
}

void mouse_position(float* x, float* y) {
    float mat[6], inv_mat[6];
    nvgCurrentTransform(vg, mat);
    nvgTransformInverse(inv_mat, mat);
    nvgTransformPoint(x, y, inv_mat, mouse_state.x, mouse_state.y);
}

void mouse_movement(float* x, float* y, float* dx, float* dy) {
    float mat[6], inv_mat[6];
    nvgCurrentTransform(vg, mat);
    nvgTransformInverse(inv_mat, mat);
    
    float ix, iy;
    nvgTransformPoint(x, y, inv_mat, mouse_state.x, mouse_state.y);
    nvgTransformPoint(&ix, &iy, inv_mat, mouse_state.initial_x, mouse_state.initial_y);
    
    *dx = *x - ix;
    *dy = *y - iy;
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
    return (key_state.character != NULL || key_state.key > 0);
}

bool has_key_event(void* identifier) {
    return (focus_state.focus_new == identifier) && (key_state.character != NULL || key_state.key > 0);
}

void consume_key_event() {
    key_state = { 0 };
}

void repeat_key_event() {
    key_state_queue.insert(key_state_queue.begin(), key_state);
    key_state = { 0 };
}

const char* get_clipboard_string() {
    if (get_clipboard_string_proc) {
        return get_clipboard_string_proc();
    } else {
        return NULL;
    }
}

void set_clipboard_string(const char* string) {
    if (set_clipboard_string_proc) {
        set_clipboard_string_proc(string);
    }
}

// Cursor state
void set_cursor(Cursor cursor) {
    cursor_state_new = cursor;
}

}
