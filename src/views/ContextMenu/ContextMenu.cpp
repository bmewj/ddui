//
//  ContextMenu.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 11/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "ContextMenu.hpp"
#include <ddui/core>
#include <ddui/util/entypo>
#include <ddui/views/ScrollArea>
#include <nanovg.h>

namespace ContextMenu {

using namespace ddui;

struct ContextMenuState {
    ContextMenuState();

    bool open;
    void* identifier;
    int action_pressing;
    int action;
    float x, y;
    std::vector<Item> items;

    ScrollArea::ScrollAreaState scroll_area_state;
};

ContextMenuState::ContextMenuState() {
    open = false;
    identifier = NULL;
}

static constexpr float ITEM_HEIGHT = 28;
static constexpr float ITEM_FONT_SIZE = 16;
static constexpr float CHECK_FONT_SIZE = 32;
static constexpr const char* ITEM_FONT_FACE = "regular";
static Color ITEM_TEXT_COLOR = rgb(0x000000);
static Color ITEM_HOVER_COLOR = rgb(0xc8c8c8);
static Color ITEM_PRESS_COLOR = rgb(0xa0a0a0);
static Color BG_COLOR = rgb(0xffffff);
static constexpr float PADDING_TOP = 4;
static constexpr float PADDING_BOTTOM = 4;

static constexpr float PADDING_HORIZONTAL = 9;
static constexpr float PADDING_LEFT = 32;
static constexpr float PADDING_RIGHT = 16;

static ContextMenuState state;

void update(std::function<void()> inner_update) {
    // Step 1. Process mouse input
    void* identifier = state.identifier;
    float menu_width, menu_height, x, y;
    if (state.open) {
        menu_height = PADDING_TOP + ITEM_HEIGHT * state.items.size() + PADDING_BOTTOM;

        font_face(ITEM_FONT_FACE);
        font_size(ITEM_FONT_SIZE);

        float bounds[4];
        float max_text_width = 0;
        for (auto& item : state.items) {
          text_bounds(0, 0, item.label.c_str(), 0, bounds);
          auto width = bounds[2] - bounds[0];
          if (max_text_width < width) {
              max_text_width = width;
          }
        }
      
        menu_width = PADDING_LEFT + max_text_width + PADDING_RIGHT;
        
        // Adjust menu position to fit in screen
        from_global_position(&x, &y, state.x, state.y);
        if (x + menu_width > view.width && menu_width < view.width) {
            x = view.width - menu_width;
        }

        // Handle context menu dismissal
        if (mouse_hit(0, 0, view.width, view.height) &&
            !mouse_hit(x, y, menu_width, menu_height)) {
            mouse_hit_accept();
            state.open = false;
            state.action = -1;
        }

        // Handle item press
        auto y2 = y - state.scroll_area_state.scroll_y + PADDING_TOP;
        for (int i = 0; i < state.items.size(); ++i) {
            if (mouse_hit(x, y2, menu_width, ITEM_HEIGHT)) {
                mouse_hit_accept();
                state.action_pressing = i;
                break;
            }
          
            y2 += ITEM_HEIGHT;
        }

        // Handle item release
        if (!mouse_state.pressed && state.action_pressing != -1) {
            state.open = false;

            auto y3 = y + PADDING_TOP + state.action_pressing * ITEM_HEIGHT - state.scroll_area_state.scroll_y;
            if (mouse_over(x, y3, menu_width, ITEM_HEIGHT)) {
                state.action = state.action_pressing;
            } else {
                state.action = -1;
            }
        }

        // Handle border clicking
        if (mouse_hit(x, y, menu_width, menu_height)) {
            mouse_hit_accept();
            state.action_pressing = -1;
        }

    }

    // Step 2. Draw child content
    inner_update();

    // Step 3. Draw context menu
    if (state.open && identifier == state.identifier) {

        auto width = view.width - x;
        if (width > menu_width) {
            width = menu_width;
        }

        auto height = view.height - y;
        if (height > menu_height) {
            height = menu_height;
        }

        sub_view(x, y, width, height);
        ScrollArea::update(&state.scroll_area_state, menu_width, menu_height, [&]() {
        
            // Background
            begin_path();
            fill_color(rgb(0xffffff));
            rect(0, 0, view.width, view.height);
            fill();
          
            // Items
            float ascender, descender, line_height;
            font_face(ITEM_FONT_FACE);
            font_size(ITEM_FONT_SIZE);
            text_metrics(&ascender, &descender, &line_height);
          
            float y = PADDING_TOP;
            float text_y = (ITEM_HEIGHT - line_height) / 2 + ascender;
            float text_x = PADDING_LEFT;
            for (int i = 0; i < state.items.size(); ++i) {
                int hover_state = 0;

                if (mouse_over(0, y, menu_width, ITEM_HEIGHT)) {
                    hover_state = 1;
                    set_cursor(CURSOR_POINTING_HAND);
                }
              
                if (state.action_pressing == i) {
                    hover_state = 2;
                  
                    float mx, my;
                    mouse_position(&mx, &my);
                    if (mx < x || mx > x + menu_width ||
                        my < y || my > y + ITEM_HEIGHT) {
                        --hover_state;
                    }
                }

                if (hover_state > 0) {
                    begin_path();
                    fill_color(hover_state == 2 ? ITEM_PRESS_COLOR : ITEM_HOVER_COLOR);
                    rect(0, y, menu_width, ITEM_HEIGHT);
                    fill();
                }
              
                fill_color(ITEM_TEXT_COLOR);
                text(text_x, y + text_y, state.items[i].label.c_str(), 0);
            
                y += ITEM_HEIGHT;
            }

            font_size(CHECK_FONT_SIZE);
            font_face("entypo");
            fill_color(ITEM_TEXT_COLOR);
            text_metrics(&ascender, &descender, &line_height);

            y = PADDING_TOP + (ascender + ITEM_HEIGHT) / 2 - 4;
            for (int i = 0; i < state.items.size(); ++i) {
                if (state.items[i].checked) {
                    text(PADDING_HORIZONTAL, y, entypo::CHECK_MARK, NULL);
                }

                y += ITEM_HEIGHT;
            }

        });
        restore();
          
    }
}

int process_action(void* identifier) {
    if (!state.open && state.identifier == identifier) {
        state.identifier = NULL;
        return state.action;
    }

    return -1;
}

void show(void* identifier, float x, float y, std::vector<Item> items) {
    state.open = true;
    state.identifier = identifier;
    state.action_pressing = -1;
    state.action = -1;
    to_global_position(&state.x, &state.y, x, y);
    state.items = std::move(items);
}

bool is_showing(void* identifier) {
    return (state.open && state.identifier == identifier);
}

}
