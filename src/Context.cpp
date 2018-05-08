//
//  Context.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 11/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "Context.hpp"

Context child_context(Context ctx, int x, int y, int width, int height) {
    auto child_ctx = ctx;
    nvgSave(child_ctx.vg);
    nvgTranslate(child_ctx.vg, x, y);
    child_ctx.x += x;
    child_ctx.y += y;
    child_ctx.width = width;
    child_ctx.height = height;

    return child_ctx;
}

bool mouse_hit(Context ctx, int x1, int y1, int width, int height) {
    int x2 = x1 + width;
    int y2 = y1 + height;
    return (
        ctx.mouse->pressed && !ctx.mouse->accepted &&
        (ctx.clip.x1 < ctx.mouse->initial_x) && (ctx.mouse->initial_x <= ctx.clip.x2) &&
        (ctx.clip.y1 < ctx.mouse->initial_y) && (ctx.mouse->initial_y <= ctx.clip.y2) &&
        (ctx.x + x1 < ctx.mouse->initial_x) && (ctx.mouse->initial_x <= ctx.x + x2) &&
        (ctx.y + y1 < ctx.mouse->initial_y) && (ctx.mouse->initial_y <= ctx.y + y2)
    );
}

bool mouse_hit_secondary(Context ctx, int x1, int y1, int width, int height) {
    int x2 = x1 + width;
    int y2 = y1 + height;
    return (
        ctx.mouse->pressed_secondary && !ctx.mouse->accepted &&
        (ctx.clip.x1 < ctx.mouse->initial_x) && (ctx.mouse->initial_x <= ctx.clip.x2) &&
        (ctx.clip.y1 < ctx.mouse->initial_y) && (ctx.mouse->initial_y <= ctx.clip.y2) &&
        (ctx.x + x1 < ctx.mouse->initial_x) && (ctx.mouse->initial_x <= ctx.x + x2) &&
        (ctx.y + y1 < ctx.mouse->initial_y) && (ctx.mouse->initial_y <= ctx.y + y2)
    );
}

bool mouse_over(Context ctx, int x1, int y1, int width, int height) {
    int x2 = x1 + width;
    int y2 = y1 + height;
    return (
        !ctx.mouse->pressed && !ctx.mouse->pressed_secondary &&
        (ctx.clip.x1 < ctx.mouse->x) && (ctx.mouse->x <= ctx.clip.x2) &&
        (ctx.clip.y1 < ctx.mouse->y) && (ctx.mouse->y <= ctx.clip.y2) &&
        (ctx.x + x1 < ctx.mouse->x) && (ctx.mouse->x <= ctx.x + x2) &&
        (ctx.y + y1 < ctx.mouse->y) && (ctx.mouse->y <= ctx.y + y2)
    );
}
