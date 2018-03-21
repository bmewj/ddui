//
//  ItemArranger.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 21/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "ItemArranger.hpp"

namespace ItemArranger {

static void button(State* state, Context ctx, int* x, int ascender, int button_height, int index);

void update(State* state, Context ctx) {

    // Handle mouse dropping input
    if (state->active_index != -1 && state->drop_point != -1 && !ctx.mouse->pressed) {
        state->model->reorder(state->active_index, state->drop_point);
        state->active_index = -1;
        state->drop_point = -1;
    }
  
    // Correct scroll_x
    state->scroll_x += ctx.mouse->scroll_dx + ctx.mouse->scroll_dy;
    if (state->scroll_x > state->content_width - ctx.width) {
        state->scroll_x = state->content_width - ctx.width;
    }
    if (state->scroll_x < 0) {
        state->scroll_x = 0;
    }

    nvgFontFace(ctx.vg, state->font_face);
    nvgFontSize(ctx.vg, state->text_size);
  
    float ascender, descender, line_height;
    nvgTextMetrics(ctx.vg, &ascender, &descender, &line_height);
  
    int button_height = line_height + 2 * state->v_padding;
    int x = 0;
  
    state->content_height = button_height;
  
    std::vector<int> button_x, button_width;
  
    // Prepare clip-region
    nvgSave(ctx.vg);
    nvgScissor(ctx.vg, 0, 0, ctx.width, ctx.height);
    nvgTranslate(ctx.vg, -state->scroll_x, 0);
  
    auto child_ctx = ctx;
    child_ctx.x -= state->scroll_x;
    child_ctx.width = state->content_width > ctx.width ? state->content_width : ctx.width;
    child_ctx.clip.x1 = ctx.x;
    child_ctx.clip.y1 = ctx.y;
    child_ctx.clip.x2 = ctx.x + ctx.width;
    child_ctx.clip.y2 = ctx.y + ctx.height;
  
    for (int i = 0; i < state->model->count(); ++i) {
        button_x.push_back(x);
        button(state, child_ctx, &x, ascender, button_height, i);
        button_width.push_back(x - button_x.back());
        x += state->h_spacing;
    }
  
    state->content_width = x - state->h_spacing;
    button_x.push_back(x);
  
    nvgRestore(ctx.vg);
  
    if (state->active_index != -1) {
        // Draw button dragging
    
        nvgSave(ctx.vg);
        nvgTranslate(ctx.vg, -state->scroll_x, 0);
    
        *ctx.cursor = CURSOR_CLOSED_HAND;
        int dx = ctx.mouse->x - ctx.mouse->initial_x;
      
        if (state->drop_point != -1 || dx != 0) {
            // Find drop point
            {
                auto x = button_x[state->active_index] + button_width[state->active_index] / 2 + dx;
              
                state->drop_point = 0;
                for (int i = state->model->count() - 1; i > 0; --i) {
                    if (x > button_x[i]) {
                        state->drop_point = i;
                        break;
                    }
                }
            }
          
            // Draw drop point
            {
                int x;
                if (state->active_index == state->drop_point ||
                    state->active_index == state->drop_point - 1) {
                    x = button_x[state->active_index] + button_width[state->active_index] / 2;
                } else {
                    x = button_x[state->drop_point] - state->h_spacing / 2;
                }
            
                nvgBeginPath(ctx.vg);
                nvgStrokeColor(ctx.vg, state->color_insertion_point);
                nvgStrokeWidth(ctx.vg, 2.0);
                nvgMoveTo(ctx.vg, x, - state->overshoot);
                nvgLineTo(ctx.vg, x, button_height + state->overshoot);
                nvgStroke(ctx.vg);
            }
          
            // Draw dragged block
            {
                nvgGlobalAlpha(ctx.vg, 0.8);
                auto label = state->model->label(state->active_index).c_str();
                auto enabled = state->model->get_enabled(state->active_index);
              
                auto bg = enabled ? state->color_background_enabled : state->color_background_disabled;
                auto fg = enabled ? state->color_text_enabled : state->color_text_disabled;
              
                auto x = button_x[state->active_index] + dx;
                auto width = button_width[state->active_index];
              
                // Background
                nvgBeginPath(ctx.vg);
                nvgFillColor(ctx.vg, bg);
                nvgRoundedRect(ctx.vg, x, 0, width, button_height, state->border_radius);
                nvgFill(ctx.vg);
              
                // Text
                nvgFillColor(ctx.vg, fg);
                nvgText(ctx.vg, x + state->h_padding, state->v_padding + ascender, label, 0);
                nvgGlobalAlpha(ctx.vg, 1.0);
            }
        }
      
        nvgRestore(ctx.vg);
    }
}

void button(State* state, Context ctx, int* x, int ascender, int button_height, int index) {
    auto label = state->model->label(index).c_str();
    auto enabled = state->model->get_enabled(index);

    // Measure button size
    float bounds[4];
    nvgTextBounds(ctx.vg, 0, 0, label, 0, bounds);
    int text_width = bounds[2];
    int button_width = text_width + 2 * state->h_padding;
  
    // Process mouse events
    if (state->active_index == index && !ctx.mouse->pressed) {
        if (mouse_over(ctx, *x, 0, button_width, button_height)) {
            enabled = !enabled;
            state->model->set_enabled(index, enabled);
        }
        state->active_index = -1;
    }
    if (mouse_hit(ctx, *x, 0, button_width, button_height)) {
        *ctx.cursor = CURSOR_CLOSED_HAND;
        ctx.mouse->accepted = true;
        state->active_index = index;
    }
  
    NVGcolor bg, fg;
    if (state->active_index == index && state->drop_point != -1) {
        bg = state->color_background_vacant;
        fg = nvgRGBA(0, 0, 0, 0);
    } else {
        bg = enabled ? state->color_background_enabled : state->color_background_disabled;
        fg = enabled ? state->color_text_enabled : state->color_text_disabled;
    }
  
    // Background
    nvgBeginPath(ctx.vg);
    nvgFillColor(ctx.vg, bg);
    nvgRoundedRect(ctx.vg, *x, 0, button_width, button_height, state->border_radius);
    nvgFill(ctx.vg);
  
    // Text
    nvgFillColor(ctx.vg, fg);
    nvgText(ctx.vg, *x + state->h_padding, state->v_padding + ascender, label, 0);
  
    // Hover highlight
    if (mouse_over(ctx, *x, 0, button_width, button_height)) {
        *ctx.cursor = CURSOR_OPEN_HAND;
        nvgBeginPath(ctx.vg);
        nvgFillColor(ctx.vg, nvgRGBAf(1.0, 1.0, 1.0, state->hover_highlight_opacity));
        nvgRoundedRect(ctx.vg, *x, 0, button_width, button_height, state->border_radius);
        nvgFill(ctx.vg);
    }
  
    *x += button_width;
}

}
