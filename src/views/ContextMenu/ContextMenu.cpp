//
//  ContextMenu.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 11/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "ContextMenu.hpp"
#include <ddui/views/DropDownMenu>

namespace ContextMenu {

ContextMenuState::ContextMenuState() {
    open = false;
    identifier = NULL;
}

static constexpr int ITEM_HEIGHT = 28;
static constexpr int ITEM_FONT_SIZE = 16;
static constexpr int CHECK_FONT_SIZE = 32;
static constexpr const char* ITEM_FONT_FACE = "regular";
static NVGcolor ITEM_TEXT_COLOR = nvgRGB(0, 0, 0);
static NVGcolor ITEM_HOVER_COLOR = nvgRGB(200, 200, 200);
static NVGcolor ITEM_PRESS_COLOR = nvgRGB(150, 150, 150);
static NVGcolor BG_COLOR = nvgRGB(255, 255, 255);
static constexpr int PADDING_TOP = 4;
static constexpr int PADDING_BOTTOM = 4;

static constexpr int PADDING_HORIZONTAL = 9;
static constexpr int PADDING_LEFT = 32;
static constexpr int PADDING_RIGHT = 16;

void update(ContextMenuState* state, Context ctx, std::function<void(Context)> inner_update) {
    // Step 1. Process mouse input
    void* identifier = state->identifier;
    int menu_width, menu_height;
    if (state->open) {
        menu_height = PADDING_TOP + ITEM_HEIGHT * state->items.size() + PADDING_BOTTOM;

        nvgFontFace(ctx.vg, ITEM_FONT_FACE);
        nvgFontSize(ctx.vg, ITEM_FONT_SIZE);

        float bounds[4];
        int max_text_width = 0;
        for (auto& item : state->items) {
          nvgTextBounds(ctx.vg, 0, 0, item.label.c_str(), 0, bounds);
          int width = (int)(bounds[2] - bounds[0]);
          if (width > max_text_width) {
              max_text_width = width;
          }
        }
      
        menu_width = PADDING_LEFT + max_text_width + PADDING_RIGHT;

        // Handle context menu dismissal
        if (mouse_hit(ctx, 0, 0, ctx.width, ctx.height) &&
            !mouse_hit(ctx, state->x, state->y, menu_width, menu_height)) {
            ctx.mouse->accepted = true;
            state->open = false;
            state->action = -1;
        }

        // Handle item press
        int y = state->y + PADDING_TOP;
        for (int i = 0; i < state->items.size(); ++i) {
            if (mouse_hit(ctx, state->x, y, menu_width, ITEM_HEIGHT)) {
                ctx.mouse->accepted = true;
                state->action_pressing = i;
                break;
            }
          
            y += ITEM_HEIGHT;
        }

        // Handle item release
        if (!ctx.mouse->pressed && state->action_pressing != -1) {
            state->open = false;

            int y = state->y + PADDING_TOP + state->action_pressing * ITEM_HEIGHT;
            if (mouse_over(ctx, state->x, y, menu_width, ITEM_HEIGHT)) {
                state->action = state->action_pressing;
            } else {
                state->action = -1;
            }
        }
      
        // Handle border clicking
        if (mouse_hit(ctx, state->x, state->y, menu_width, menu_height)) {
            ctx.mouse->accepted = true;
            state->action_pressing = -1;
        }
      
    }

    // Step 2. Draw child content
    auto child_ctx = ctx;
    child_ctx.context_menu_state = state;
    inner_update(child_ctx);

    // Step 3. Draw context menu
    if (state->open && identifier == state->identifier) {

        int width = ctx.width - state->x;
        if (width > menu_width) {
            width = menu_width;
        }
      
        int height = ctx.height - state->y;
        if (height > menu_height) {
            height = menu_height;
        }
      
        auto child_ctx = child_context(ctx, state->x, state->y, width, height);
        ScrollArea::update(&state->scroll_area_state, child_ctx, menu_width, menu_height, [&](Context ctx) {
        
            // Background
            nvgBeginPath(ctx.vg);
            nvgFillColor(ctx.vg, nvgRGB(255, 255, 255));
            nvgRect(ctx.vg, 0, 0, ctx.width, ctx.height);
            nvgFill(ctx.vg);
          
            // Items
            float ascender, descender, line_height;
            nvgFontFace(ctx.vg, ITEM_FONT_FACE);
            nvgFontSize(ctx.vg, ITEM_FONT_SIZE);
            nvgTextMetrics(ctx.vg, &ascender, &descender, &line_height);
          
            int y = PADDING_TOP;
            int text_y = (ITEM_HEIGHT - (int)line_height) / 2 + (int)ascender;
            int text_x = PADDING_LEFT;
            for (int i = 0; i < state->items.size(); ++i) {
                int hover_state = 0;

                if (mouse_over(ctx, 0, y, menu_width, ITEM_HEIGHT)) {
                    hover_state = 1;
                    *ctx.cursor = CURSOR_POINTING_HAND;
                }
              
                if (state->action_pressing == i) {
                    hover_state = 2;
                  
                    if (ctx.mouse->x < state->x || ctx.mouse->x > state->x + menu_width ||
                        ctx.mouse->y < y || ctx.mouse->y > y + ITEM_HEIGHT) {
                        --hover_state;
                    }
                }

                if (hover_state > 0) {
                    nvgBeginPath(ctx.vg);
                    nvgFillColor(ctx.vg, hover_state == 2 ? ITEM_PRESS_COLOR : ITEM_HOVER_COLOR);
                    nvgRect(ctx.vg, 0, y, menu_width, ITEM_HEIGHT);
                    nvgFill(ctx.vg);
                }
              
                nvgFillColor(ctx.vg, ITEM_TEXT_COLOR);
                nvgText(ctx.vg, text_x, y + text_y, state->items[i].label.c_str(), 0);
            
                y += ITEM_HEIGHT;
            }

            nvgFontSize(ctx.vg, CHECK_FONT_SIZE);
            nvgFontFace(ctx.vg, "entypo");
            nvgFillColor(ctx.vg, ITEM_TEXT_COLOR);
            nvgTextMetrics(ctx.vg, &ascender, &descender, &line_height);

            y = PADDING_TOP + (ascender + ITEM_HEIGHT) / 2 - 4;
            for (int i = 0; i < state->items.size(); ++i) {
                if (state->items[i].checked) {
                    nvgText(ctx.vg, PADDING_HORIZONTAL, y, entypo::CHECK_MARK, NULL);
                }

                y += ITEM_HEIGHT;
            }

        });
        nvgRestore(ctx.vg);
          
    }
}

int process_action(Context ctx, void* identifier) {
    return DropDownMenu::process_action(ctx, identifier);
}

void show(Context ctx, void* identifier, std::vector<Item> items) {
    std::vector<DropDownMenu::Item> items2;
    items2.reserve(items.size());
    for (auto& item : items) {
        DropDownMenu::Item item2;
        item2.label = std::move(item.label);
        item2.checked = item.checked;
        items2.push_back(std::move(item2));
    }
    DropDownMenu::show(ctx, identifier, ctx.mouse->x, ctx.mouse->y, std::move(items2));
}

}
