//
//  core.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 23/08/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_core_hpp
#define ddui_core_hpp

#include <functional>

namespace ddui {

// Types
struct Color {
    float r, g, b, a;
};
struct Paint {
    float xform[6];
    float extent[2];
    float radius;
    float feather;
    Color inner_color;
    Color outer_color;
    int image;
};

namespace align {
    // Horizontal alignment
    constexpr int LEFT     = 1 << 0;
    constexpr int CENTER   = 1 << 1;
    constexpr int RIGHT    = 1 << 2;

    // Vertical alignment
    constexpr int TOP      = 1 << 3;
    constexpr int MIDDLE   = 1 << 4;
    constexpr int BOTTOM   = 1 << 5;
    constexpr int BASELINE = 1 << 6;
}

namespace direction {
    constexpr int COUNTER_CLOCKWISE = 1;
    constexpr int CLOCKWISE = 2;
}

#undef MOD_SHIFT
#undef MOD_CONTROL
#undef MOD_ALT
#undef MOD_SUPER

namespace keyboard {
    constexpr int ACTION_PRESS   = 1;
    constexpr int ACTION_RELEASE = 0;
    constexpr int ACTION_REPEAT  = 2;

    constexpr int MOD_SHIFT   = 0x0001;
    constexpr int MOD_CONTROL = 0x0002;
    constexpr int MOD_ALT     = 0x0004;
    constexpr int MOD_SUPER   = 0x0008;
    
    constexpr int MOD_COMMAND = 0x000a;

    constexpr int KEY_UNKNOWN = 1;
    constexpr int KEY_SPACE = 32;
    constexpr int KEY_APOSTROPHE = 39; /* ' */
    constexpr int KEY_COMMA = 44; /* , */
    constexpr int KEY_MINUS = 45; /* - */
    constexpr int KEY_PERIOD = 46; /* . */
    constexpr int KEY_SLASH = 47; /* / */
    constexpr int KEY_0 = 48;
    constexpr int KEY_1 = 49;
    constexpr int KEY_2 = 50;
    constexpr int KEY_3 = 51;
    constexpr int KEY_4 = 52;
    constexpr int KEY_5 = 53;
    constexpr int KEY_6 = 54;
    constexpr int KEY_7 = 55;
    constexpr int KEY_8 = 56;
    constexpr int KEY_9 = 57;
    constexpr int KEY_SEMICOLON = 59; /* ; */
    constexpr int KEY_EQUAL = 61; /* = */
    constexpr int KEY_A = 65;
    constexpr int KEY_B = 66;
    constexpr int KEY_C = 67;
    constexpr int KEY_D = 68;
    constexpr int KEY_E = 69;
    constexpr int KEY_F = 70;
    constexpr int KEY_G = 71;
    constexpr int KEY_H = 72;
    constexpr int KEY_I = 73;
    constexpr int KEY_J = 74;
    constexpr int KEY_K = 75;
    constexpr int KEY_L = 76;
    constexpr int KEY_M = 77;
    constexpr int KEY_N = 78;
    constexpr int KEY_O = 79;
    constexpr int KEY_P = 80;
    constexpr int KEY_Q = 81;
    constexpr int KEY_R = 82;
    constexpr int KEY_S = 83;
    constexpr int KEY_T = 84;
    constexpr int KEY_U = 85;
    constexpr int KEY_V = 86;
    constexpr int KEY_W = 87;
    constexpr int KEY_X = 88;
    constexpr int KEY_Y = 89;
    constexpr int KEY_Z = 90;
    constexpr int KEY_LEFT_BRACKET = 91; /* [ */
    constexpr int KEY_BACKSLASH = 92; /* \ */
    constexpr int KEY_RIGHT_BRACKET = 93; /* ] */
    constexpr int KEY_GRAVE_ACCENT = 96; /* ` */
    constexpr int KEY_WORLD_1 = 161; /* non-US #1 */
    constexpr int KEY_WORLD_2 = 162; /* non-US #2 */
    constexpr int KEY_ESCAPE = 256;
    constexpr int KEY_ENTER = 257;
    constexpr int KEY_TAB = 258;
    constexpr int KEY_BACKSPACE = 259;
    constexpr int KEY_INSERT = 260;
    constexpr int KEY_DELETE = 261;
    constexpr int KEY_RIGHT = 262;
    constexpr int KEY_LEFT = 263;
    constexpr int KEY_DOWN = 264;
    constexpr int KEY_UP = 265;
    constexpr int KEY_PAGE_UP = 266;
    constexpr int KEY_PAGE_DOWN = 267;
    constexpr int KEY_HOME = 268;
    constexpr int KEY_END = 269;
    constexpr int KEY_CAPS_LOCK = 280;
    constexpr int KEY_SCROLL_LOCK = 281;
    constexpr int KEY_NUM_LOCK = 282;
    constexpr int KEY_PRINT_SCREEN = 283;
    constexpr int KEY_PAUSE = 284;
    constexpr int KEY_F1 = 290;
    constexpr int KEY_F2 = 291;
    constexpr int KEY_F3 = 292;
    constexpr int KEY_F4 = 293;
    constexpr int KEY_F5 = 294;
    constexpr int KEY_F6 = 295;
    constexpr int KEY_F7 = 296;
    constexpr int KEY_F8 = 297;
    constexpr int KEY_F9 = 298;
    constexpr int KEY_F10 = 299;
    constexpr int KEY_F11 = 300;
    constexpr int KEY_F12 = 301;
    constexpr int KEY_F13 = 302;
    constexpr int KEY_F14 = 303;
    constexpr int KEY_F15 = 304;
    constexpr int KEY_F16 = 305;
    constexpr int KEY_F17 = 306;
    constexpr int KEY_F18 = 307;
    constexpr int KEY_F19 = 308;
    constexpr int KEY_F20 = 309;
    constexpr int KEY_F21 = 310;
    constexpr int KEY_F22 = 311;
    constexpr int KEY_F23 = 312;
    constexpr int KEY_F24 = 313;
    constexpr int KEY_F25 = 314;
    constexpr int KEY_KP_0 = 320;
    constexpr int KEY_KP_1 = 321;
    constexpr int KEY_KP_2 = 322;
    constexpr int KEY_KP_3 = 323;
    constexpr int KEY_KP_4 = 324;
    constexpr int KEY_KP_5 = 325;
    constexpr int KEY_KP_6 = 326;
    constexpr int KEY_KP_7 = 327;
    constexpr int KEY_KP_8 = 328;
    constexpr int KEY_KP_9 = 329;
    constexpr int KEY_KP_DECIMAL = 330;
    constexpr int KEY_KP_DIVIDE = 331;
    constexpr int KEY_KP_MULTIPLY = 332;
    constexpr int KEY_KP_SUBTRACT = 333;
    constexpr int KEY_KP_ADD = 334;
    constexpr int KEY_KP_ENTER = 335;
    constexpr int KEY_KP_EQUAL = 336;
    constexpr int KEY_LEFT_SHIFT = 340;
    constexpr int KEY_LEFT_CONTROL = 341;
    constexpr int KEY_LEFT_ALT = 342;
    constexpr int KEY_LEFT_SUPER = 343;
    constexpr int KEY_RIGHT_SHIFT = 344;
    constexpr int KEY_RIGHT_CONTROL = 345;
    constexpr int KEY_RIGHT_ALT = 346;
    constexpr int KEY_RIGHT_SUPER = 347;
    constexpr int KEY_MENU = 348;
    constexpr int KEY_LAST = KEY_MENU;
}

namespace image {
    constexpr int GENERATE_MIPMAPS = 1<<0; // Generate mipmaps during creation of the image.
    constexpr int REPEAT_X         = 1<<1; // Repeat image in X direction.
    constexpr int REPEAT_Y         = 1<<2; // Repeat image in Y direction.
    constexpr int FLIP_Y           = 1<<3; // Flips (inverses) image in Y direction when rendered.
    constexpr int PREMULTIPLIED    = 1<<4; // Image data has premultiplied alpha.
    constexpr int NEAREST          = 1<<5; // Image interpolation is Nearest instead Linear
}

struct GlyphPosition {
    const char* str;    // Position of the glyph in the input string.
    float x;            // The x-coordinate of the logical glyph position.
    float minx, maxx;   // The bounds of the glyph shape.
};

struct MouseState {
    int x, y;
    bool pressed, pressed_secondary, accepted;
    int initial_x, initial_y;
    int scroll_dx, scroll_dy;
};

struct KeyState {
    const char* character;
    int action;
    int key;
    int mods;
};

enum Cursor {
    CURSOR_ARROW,
    CURSOR_IBEAM,
    CURSOR_CROSS_HAIR,
    CURSOR_POINTING_HAND,
    CURSOR_CLOSED_HAND,
    CURSOR_OPEN_HAND,
    CURSOR_HORIZONTAL_RESIZE,
    CURSOR_VERTICAL_RESIZE,
  
    CURSOR_COUNT
};

// Setup
bool init();
void set_post_empty_message_proc(std::function<void()> proc);
void set_get_clipboard_string_proc(std::function<const char*()> proc);
void set_set_clipboard_string_proc(std::function<void(const char*)> proc);
void set_set_cursor_proc(std::function<void(Cursor)> proc);

// Teardown
void terminate();

// User input
void input_key(int key, int scancode, int action, int mods);
void input_character(unsigned int codepoint);
void input_mouse_position(float x, float y);
void input_mouse_button(int button, int action, int mods);
void input_scroll(float offset_x, float offset_y);

// Frame management
void update(float width, float height, float pixel_ratio, std::function<void()> update_proc);
void repaint(const char* reason);
void set_immediate(std::function<void()> callback);
void set_post_update(std::function<void()> callback);

// Color utils
Color rgb(unsigned char r, unsigned char g, unsigned char b);
Color rgb(unsigned int rgb);
Color rgba(unsigned char r, unsigned char g, unsigned char b, float a);
Color rgba(unsigned int rgb, float a);
Color color_interpolate(Color a, Color b, float ratio);

// State Handling
struct Viewport {
    float width, height;
    struct {
        float x1, y1, x2, y2;
    } clip;
};
extern Viewport view;
void save();
void restore();
void reset();
void sub_view(float x, float y, float width, float height);

// Render styles
// void shape_anti_alias(bool enabled);
void stroke_color(Color color);
void stroke_paint(Paint paint);
void fill_color(Color color);
void fill_paint(Paint paint);
// void miter_limit(float limit);
void stroke_width(float size);
// void line_cap(int cap);
// void line_join(int join);
void global_alpha(float alpha);

// Transforms
void reset_transform();
void transform(float a, float b, float c, float d, float e, float f);
void translate(float x, float y);
void rotate(float angle);
void skew_x(float angle);
void skew_y(float angle);
void scale(float x, float y);
void to_global_position(float* gx, float* gy, float x, float y);
void from_global_position(float* x, float* y, float gx, float gy);

// Images
int create_image_from_file(const char* filename, int image_flags);
int create_image_from_rgba(int w, int h, int image_flags, const unsigned char* data);
void update_image(int image_id, const unsigned char* data);
void image_size(int image_id, int* w, int* h);
void delete_image(int image_id);

// Paints
Paint linear_gradient(float sx, float sy, float ex, float ey, Color icol, Color ocol);
Paint box_gradient(float x, float y, float w, float h, float r, float f, Color icol, Color ocol);
Paint radial_gradient(float cx, float cy, float inr, float outr, Color icol, Color ocol);
Paint image_pattern(float ox, float oy, float ex, float ey, float angle, int image, float alpha);

// Clipping
void clip(float x, float y, float width, float height);
void get_clip_dimensions(float* width, float* height);

// Paths
void begin_path();
void move_to(float x, float y);
void line_to(float x, float y);
void bezier_to(float c1x, float c1y, float c2x, float c2y, float x, float y);
void quad_to(float cx, float cy, float x, float y);
void arc_to(float x1, float y1, float x2, float y2, float radius);
void close_path();
void path_winding(int dir);
void arc(float cx, float cy, float r, float a0, float a1, int dir);
void rect(float x, float y, float w, float h);
void rounded_rect(float x, float y, float w, float h, float r);
void rounded_rect_varying(float x, float y, float w, float h, float radTopLeft, float radTopRight, float radBottomRight, float radBottomLeft);
void ellipse(float cx, float cy, float rx, float ry);
void circle(float cx, float cy, float r);
void fill();
void stroke();

// Text
int create_font(const char* name, const char* filename);
void font_size(float size);
void text_align(int align);
void font_face(const char* font);
float text(float x, float y, const char* string, const char* end);
void text_box(float x, float y, float breakRowWidth, const char* string, const char* end);
float text_bounds(float x, float y, const char* string, const char* end, float* bounds);
void text_box_bounds(float x, float y, float breakRowWidth, const char* string, const char* end, float* bounds);
int text_glyph_positions(float x, float y, const char* string, const char* end, GlyphPosition* positions, int maxPositions);
void text_metrics(float* ascender, float* descender, float* lineh);

// Mouse state
extern MouseState mouse_state;
bool mouse_hit(float x, float y, float width, float height);
bool mouse_hit_secondary(float x, float y, float width, float height);
bool mouse_over(float x, float y, float width, float height);
void mouse_hit_accept();
void mouse_position(float* x, float* y);
void mouse_movement(float* x, float* y, float* dx, float* dy);

// Focus state
void register_focus_group(void* identifier);
bool did_focus(void* identifier);
bool did_blur(void* identifier);
bool has_focus(void* identifier);
void tab_forward();
void tab_backward();
void focus(void* identifier);
void blur();

// Keyboard state
extern KeyState key_state;
bool has_key_event();
bool has_key_event(void* identifier);
void consume_key_event();
void repeat_key_event();
const char* get_clipboard_string();
void set_clipboard_string(const char* string);

// Cursor state
void set_cursor(Cursor cursor);

// Animation
namespace animation {
    void start(void* identifier);
    void stop(void* identifier);
    bool is_animating(void* identifier);
    double get_time_elapsed(void* identifier);
    double ease_in(double completion);
    double ease_out(double completion);
    double ease_in_out(double completion);
    bool is_animating();
}

// Timers
namespace timer {
    int set_timeout(std::function<void()> callback, long time_in_ms);
    int set_interval(std::function<void()> callback, long time_in_ms);
    void clear_timeout(int timeout_id);
    void clear_interval(int interval_id);
}

}

#endif
