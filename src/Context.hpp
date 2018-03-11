//
//  Context.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 11/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_Context_hpp
#define ddui_Context_hpp

#include <nanovg.h>
#include <functional>
#include <vector>
#include "views/ContextMenu/ContextMenuState.hpp"

struct MouseState {
    int x, y;
    bool pressed, pressed_secondary, accepted;
    int initial_x, initial_y;
    int scroll_dx, scroll_dy;
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

struct Context {
    NVGcontext* vg;
    MouseState* mouse;
    Cursor* cursor;
    bool* must_repaint;
    ContextMenu::ContextMenuState* context_menu_state;

    int x, y;
    int width, height;
    int global_x, global_y;

    struct {
      int x1, y1;
      int x2, y2;
    } clip;
};

Context child_context(Context ctx, int x, int y, int width, int height);

bool mouse_hit(Context ctx, int x, int y, int width, int height);
bool mouse_hit_secondary(Context ctx, int x, int y, int width, int height);
bool mouse_over(Context ctx, int x, int y, int width, int height);

#endif
