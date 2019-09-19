//
//  ItemArranger.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 21/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "ItemArranger.hpp"

namespace ItemArranger {

using namespace ddui;

static void button(State* state, float* x, float ascender, float button_height, int index);

void update(State* state) {

    // Handle mouse dropping input
    if (state->active_index != -1 && state->drop_point != -1 && !mouse_state.pressed) {
        state->model->reorder(state->active_index, state->drop_point);
        state->active_index = -1;
        state->drop_point = -1;
    }
  
    // Correct scroll_x
    if (mouse_over(0, 0, view.width, state->content_height)) {
        state->scroll_x += mouse_state.scroll_dx + mouse_state.scroll_dy;
    }
    if (state->scroll_x > state->content_width - view.width) {
        state->scroll_x = state->content_width - view.width;
    }
    if (state->scroll_x < 0) {
        state->scroll_x = 0;
    }

    font_face(state->font_face);
    font_size(state->text_size);
    text_align(align::LEFT | align::BASELINE);
  
    float ascender, descender, line_height;
    text_metrics(&ascender, &descender, &line_height);
  
    float button_height = line_height + 2 * state->v_padding;
    float x = 0;
  
    state->content_height = button_height;
  
    std::vector<float> button_x, button_width;
  
    // Prepare clip-region
    save();
    clip(0, 0, view.width, view.height);
    sub_view(-state->scroll_x, 0, state->content_width > view.width ? state->content_width : view.width, view.height);

    for (int i = 0; i < state->model->count(); ++i) {
        button_x.push_back(x);
        button(state, &x, ascender, button_height, i);
        button_width.push_back(x - button_x.back());
        x += state->h_spacing;
    }
  
    state->content_width = x - state->h_spacing;
    button_x.push_back(x);

    restore();
    restore();
  
    if (state->active_index != -1) {
        // Draw button dragging

        save();
        translate(-state->scroll_x, 0);

        set_cursor(CURSOR_CLOSED_HAND);
        float mx, my, dx, dy;
        mouse_movement(&mx, &my, &dx, &dy);
      
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

                begin_path();
                stroke_color(state->color_insertion_point);
                stroke_width(2.0);
                move_to(x, - state->overshoot);
                line_to(x, button_height + state->overshoot);
                stroke();
            }

            // Draw dragged block
            {
                global_alpha(0.8);
                auto label = state->model->label(state->active_index);
                auto enabled = state->model->get_enabled(state->active_index);
              
                auto bg = enabled ? state->color_background_enabled : state->color_background_disabled;
                auto fg = enabled ? state->color_text_enabled : state->color_text_disabled;
              
                auto x = button_x[state->active_index] + dx;
                auto width = button_width[state->active_index];
              
                // Background
                begin_path();
                fill_color(bg);
                rounded_rect(x, 0, width, button_height, state->border_radius);
                fill();
              
                // Text
                fill_color(fg);
                text(x + state->h_padding, state->v_padding + ascender, label.c_str(), 0);
                global_alpha(1.0);
            }
        }
      
        restore();
    }
}

void button(State* state, float* x, float ascender, float button_height, int index) {
    auto label = state->model->label(index);
    auto enabled = state->model->get_enabled(index);

    // Measure button size
    float bounds[4];
    text_bounds(0, 0, label.c_str(), 0, bounds);
    auto text_width = bounds[2];
    auto button_width = text_width + 2 * state->h_padding;

    // Process mouse events
    if (state->active_index == index && !mouse_state.pressed) {
        if (mouse_over(*x, 0, button_width, button_height)) {
            enabled = !enabled;
            state->model->set_enabled(index, enabled);
        }
        state->active_index = -1;
    }
    if (mouse_hit(*x, 0, button_width, button_height)) {
        set_cursor(CURSOR_CLOSED_HAND);
        mouse_hit_accept();
        state->active_index = index;
    }

    Color bg, fg;
    if (state->active_index == index && state->drop_point != -1) {
        bg = state->color_background_vacant;
        fg = rgba(0x000000, 0.0);
    } else {
        bg = enabled ? state->color_background_enabled : state->color_background_disabled;
        fg = enabled ? state->color_text_enabled : state->color_text_disabled;
    }

    // Background
    begin_path();
    fill_color(bg);
    rounded_rect(*x, 0, button_width, button_height, state->border_radius);
    fill();

    // Text
    fill_color(fg);
    text(*x + state->h_padding, state->v_padding + ascender, label.c_str(), 0);

    // Hover highlight
    if (mouse_over(*x, 0, button_width, button_height)) {
        set_cursor(CURSOR_OPEN_HAND);
        begin_path();
        fill_color(rgba(0xffffff, state->hover_highlight_opacity));
        rounded_rect(*x, 0, button_width, button_height, state->border_radius);
        fill();
    }

    *x += button_width;
}

}
