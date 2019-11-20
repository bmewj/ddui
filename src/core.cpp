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
#include "util/get_asset_filename.hpp"
#include "profiling.hpp"
#include <vector>
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
static std::mutex repaint_mutex;
static bool is_painting, should_repaint;
static std::mutex set_immediate_mutex;
static std::vector<std::function<void()>> set_immediate_callbacks;
static std::mutex set_post_update_mutex;
static std::vector<std::function<void()>> set_post_update_callbacks;
static Cursor cursor_state_old, cursor_state_new;
MouseState mouse_state;
KeyState key_state;
Viewport view;
static std::vector<Viewport> saved_views;

// input.cpp internal functions
void pop_input_events_into_global_state();
bool has_input_events_to_process();

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

    create_font("entypo", "Entypo.ttf");
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

// Frame management
static void update_pre(float width, float height, float pixel_ratio);
static void update_post();

void update(float width, float height, float pixel_ratio, std::function<void()> update_proc) {

    #ifdef DDUI_PROFILING_ON
        profiling::frame_start();
    #endif

    // Setup GL frame
    auto frame_buffer_width  = (int)(width * pixel_ratio);
    auto frame_buffer_height = (int)(height * pixel_ratio);
    glViewport(0, 0, frame_buffer_width, frame_buffer_height);
    glClearColor(0.949f, 0.949f, 0.949f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    repaint_mutex.lock();
    is_painting = true;
    repaint_mutex.unlock();

    while (true) {
        nvgBeginFrame(vg, width, height, pixel_ratio);
        update_pre(width, height, pixel_ratio);
        update_proc();
        update_post();

        repaint_mutex.lock();
        is_painting = should_repaint;
        repaint_mutex.unlock();
        if (!is_painting) {
            break;
        }

        #ifdef DDUI_PROFILING_ON
            profiling::repaint_start();
        #endif

        nvgCancelFrame(vg);
    }

    nvgEndFrame(vg);

    #ifdef DDUI_PROFILING_ON
        profiling::frame_end();
    #endif

}

void update_pre(float width, float height, float pixel_ratio) {

    // Process all set_immediate callbacks
    while (true) {
        set_immediate_mutex.lock();
        auto callbacks = std::move(set_immediate_callbacks);
        set_immediate_mutex.unlock();
        #ifdef DDUI_PROFILING_ON
            profiling::num_set_immediates += callbacks.size();
        #endif
        for (auto& callback : callbacks) {
            callback();
        }
        if (callbacks.empty()) {
            break;
        }
    }

    // Reset should_repaint
    repaint_mutex.lock();
    should_repaint = false;
    repaint_mutex.unlock();

    // Let the animation system know that a new frame is being generated
    update_animation();

    cursor_state_new = CURSOR_ARROW;

    pop_input_events_into_global_state();

    focus_state.action = FocusState::NO_CHANGE;
    focus_state.groups.clear();

    view.width = width;
    view.height = height;
    saved_views.clear();
    saved_views.push_back(view);
}

void update_post() {

    #ifdef DDUI_PROFILING_ON
        profiling::num_repaints += 1;
    #endif

    // Reset scroll input
    mouse_state.scroll_dx = 0.0;
    mouse_state.scroll_dy = 0.0;

    // Update focus group
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
        has_input_events_to_process()) {
        should_repaint = true;
    }

    // Update cursor
    if (cursor_state_old != cursor_state_new) {
        cursor_state_old  = cursor_state_new;
        if (set_cursor_proc) {
            set_cursor_proc(cursor_state_new);
        }
    }

    // Call all post_update callbacks
    set_post_update_mutex.lock();
    auto callbacks = std::move(set_post_update_callbacks);
    set_post_update_mutex.unlock();
    for (auto& callback : callbacks) {
        callback();
    }
}

void repaint(const char* reason) {
    repaint_mutex.lock();
    if (is_painting) {
        should_repaint = true;

        #ifdef DDUI_PROFILING_ON
            if (reason != NULL) {
                profiling::repaint_reason(reason);
            }
        #endif

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
    repaint(NULL);
}

void set_post_update(std::function<void()> callback) {
    set_post_update_mutex.lock();
    set_post_update_callbacks.push_back(std::move(callback));
    set_post_update_mutex.unlock();
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
Color color_interpolate(Color a, Color b, float ratio) {
    Color color;
    color.r = a.r + ratio * (b.r - a.r);
    color.g = a.g + ratio * (b.g - a.g);
    color.b = a.b + ratio * (b.b - a.b);
    color.a = a.a + ratio * (b.a - a.a);
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

// Images
int create_image_from_file(const char* filename, int image_flags) {
    return nvgCreateImage(vg, filename, image_flags);
}

int create_image_from_rgba(int w, int h, int image_flags, const unsigned char* data) {
    return nvgCreateImageRGBA(vg, w, h, image_flags, data);
}

void update_image(int image_id, const unsigned char* data) {
    return nvgUpdateImage(vg, image_id, data);
}

void image_size(int image_id, int* w, int* h) {
    return nvgImageSize(vg, image_id, w, h);
}

void delete_image(int image_id) {
    return nvgDeleteImage(vg, image_id);
}

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

static NVGscissor* get_scissor();
void get_clip_dimensions(float* width, float* height) {
    auto scissor = get_scissor();
    *width  = scissor->extent[0] * 2.0;
    *height = scissor->extent[1] * 2.0;
}

bool rect_appears_in_clip_region(float x, float y, float width, float height) {
    auto scissor = get_scissor();

    // Step 1. Transform rectangle coordinates to screen coordinates
    float x1, y1, x2, y2, x3, y3, x4, y4;
    {
        float xform[6];
        nvgCurrentTransform(vg, xform);
        nvgTransformPoint(&x1, &y1, xform, x, y);                  // top-left corner
        nvgTransformPoint(&x4, &y4, xform, x + width, y + height); // bottom-right corner
        x2 = x1; y2 = y4;                                          // bottom-left corner
        x3 = x4; y3 = y1;                                          // top-right corner
    }

    // Step 2. Transform rectangle screen coordinates to scissor coordinates
    {
        float inv_xform[6];
        nvgTransformInverse(inv_xform, scissor->xform);
        nvgTransformPoint(&x1, &y1, inv_xform, x1, y1);            // top-left corner
        nvgTransformPoint(&x2, &y2, inv_xform, x2, y2);            // bottom-right corner
        nvgTransformPoint(&x3, &y3, inv_xform, x3, y3);            // bottom-left corner 
        nvgTransformPoint(&x4, &y4, inv_xform, x4, y4);            // top-right corner
    }

    // Step 3. Get the min and max bounds of the transformed points
    float x_min, x_max, y_min, y_max;
    {
        x_min = x1;
        if (x_min > x2) x_min = x2;
        if (x_min > x3) x_min = x3;
        if (x_min > x4) x_min = x4;

        x_max = x1;
        if (x_max < x2) x_max = x2;
        if (x_max < x3) x_max = x3;
        if (x_max < x4) x_max = x4;

        y_min = y1;
        if (y_min > y2) y_min = y2;
        if (y_min > y3) y_min = y3;
        if (y_min > y4) y_min = y4;

        y_max = y1;
        if (y_max < y2) y_max = y2;
        if (y_max < y3) y_max = y3;
        if (y_max < y4) y_max = y4;
    }

    // Step 4. Check if the bounded rectangle is within the clipping region extent
    return (
        (-scissor->extent[0] <= x_max && x_min < scissor->extent[0]) &&
        (-scissor->extent[1] <= y_max && y_min < scissor->extent[1])
    );
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
    auto asset_filename = get_asset_filename(filename);
    return nvgCreateFont(vg, name, asset_filename.c_str());
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
