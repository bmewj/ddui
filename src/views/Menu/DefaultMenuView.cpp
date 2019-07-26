//
//  DefaultMenuView.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 25/07/2019.
//  Copyright © 2019 Bartholomew Joyce All rights reserved.
//

#include "DefaultMenuView.hpp"
#include <ddui/core>
#include <ddui/util/draw_text_in_box>
#include <ddui/util/entypo>
#include <ddui/util/BoxShadow>

namespace {
    constexpr float ITEM_HEIGHT = 20.0;
    constexpr float BORDER_RADIUS = 4.0;
    constexpr const char* FONT_FACE = "regular";
    constexpr float FONT_SIZE = 16.0;
    constexpr float LEFT_MARGIN_WIDTH = 32.0;
    constexpr float RIGHT_MARGIN_WIDTH = 28.0;

    static auto COLOR_BG = ddui::rgb(0xffffff);
    static auto COLOR_FG = ddui::rgb(0x000000);
    static auto COLOR_BG_ACTIVE = ddui::rgb(0x5d92f1);
    static auto COLOR_FG_ACTIVE = ddui::rgb(0xffffff);
    static auto COLOR_FG_DISABLED = ddui::rgb(0xbbbbbb);
    
    constexpr float SEPARATOR_WIDTH = 1.0;
    static auto SEPARATOR_COLOR = ddui::rgb(0xdddddd);
}

Menu::IMenuView* DefaultMenuView::construct() {
    return (Menu::IMenuView*)new DefaultMenuView;
}

void DefaultMenuView::get_bounding_rect(const SubMenuState& menu, float max_width, float max_height, BoundingRect* rect) {

    float width;
    {
        ddui::font_face(FONT_FACE);
        ddui::font_size(FONT_SIZE);
        ddui::text_align(ddui::align::LEFT | ddui::align::MIDDLE);

        float max_text_width = 0.0;
        float bounds[4];
        for (const auto& item : menu.items) {
            ddui::text_bounds(0, 0, item.text.c_str(), NULL, bounds);
            float text_width = bounds[2] - bounds[0];
            if (max_text_width < text_width) {
                max_text_width = text_width;
            }
        }

        width = LEFT_MARGIN_WIDTH + max_text_width + RIGHT_MARGIN_WIDTH;
    }

    float height;
    {
        height = menu.items.size() * ITEM_HEIGHT + 2 * BORDER_RADIUS;
    }

    content_width = width;
    content_height = height;

    if (width > max_width) {
        width = max_width;
    }
    if (height > max_height) {
        height = max_height;
    }

    float max_scroll_y = content_height - height;
    if (scroll_y > max_scroll_y) {
        scroll_y = max_scroll_y;
    }

    rect->x = 0;
    rect->y = BORDER_RADIUS; // (*)
    rect->width = width;
    rect->height = height;

    // (*) The x, y coordinates in the bounds object signify the x, y offset
    // of the first item in the menu. This is used to properly align the menu
    // when it is first opened such that the selected item in the parent menu
    // is lined up with the first item of this menu.
}

void DefaultMenuView::get_item_anchors(const SubMenuState& menu, const BoundingRect& rect, int item_index, Anchor* a, Anchor* b) {
    float item_y = rect.y - scroll_y + BORDER_RADIUS + item_index * ITEM_HEIGHT;
    
    a->direction = Anchor::LEFT_TO_RIGHT;
    a->x = rect.x + rect.width;
    a->y = item_y;

    b->direction = Anchor::RIGHT_TO_LEFT;
    b->x = rect.x;
    b->y = item_y;
}

int DefaultMenuView::process_user_input(const SubMenuState& menu, const BoundingRect& rect) {
    float first_item_y = rect.y - scroll_y + BORDER_RADIUS;

    float mx, my;
    ddui::mouse_position(&mx, &my);

    auto index = (int)((my - first_item_y) / ITEM_HEIGHT);
    if (index < 0 || index >= menu.items.size()) {
        index = -1;
    }
    
    float max_scroll_y = content_height - rect.height;
    if (max_scroll_y > 0 && ddui::mouse_state.scroll_dy != 0) {
        scroll_y += ddui::mouse_state.scroll_dy;
        ddui::mouse_state.scroll_dy = 0;
    }
    if (scroll_y > max_scroll_y) {
        scroll_y = max_scroll_y;
    }
    if (scroll_y < 0) {
        scroll_y = 0;
    }

    return index;
}

void DefaultMenuView::render(const SubMenuState& menu, const BoundingRect& rect, int selected_item_index) {
    // Menu shadow
    ddui::sub_view(rect.x, rect.y, rect.width, rect.height);
    {
        ddui::BoxShadow()
            .blur(1.5)
            .border_radius(BORDER_RADIUS)
            .offset(0.5, 0.5)
            .render();
    }
    ddui::restore();

    // Menu background
    ddui::begin_path();
    ddui::rounded_rect(rect.x, rect.y, rect.width, rect.height, BORDER_RADIUS);
    ddui::fill_color(COLOR_BG);
    ddui::fill();

    ddui::save(); // a
    ddui::clip(rect.x, rect.y, rect.width, rect.height);
    ddui::translate(0, -scroll_y);

    ddui::sub_view(rect.x, rect.y + BORDER_RADIUS, rect.width, menu.items.size() * ITEM_HEIGHT); // b

    draw_items(menu, selected_item_index);

    ddui::restore(); // b
    ddui::restore(); // a
}

void DefaultMenuView::draw_items(const SubMenuState& menu, int selected_item_index) {
    float x = LEFT_MARGIN_WIDTH;
    float y = 0;
    float w = ddui::view.width - LEFT_MARGIN_WIDTH - RIGHT_MARGIN_WIDTH + 5;
    
    for (int i = 0; i < menu.items.size(); ++i) {
        const auto& item = menu.items[i];

        // Draw separator
        if (item.separator) {
            ddui::begin_path();
            ddui::stroke_color(SEPARATOR_COLOR);
            ddui::stroke_width(SEPARATOR_WIDTH);
            ddui::move_to(0, y + 0.5 * ITEM_HEIGHT);
            ddui::line_to(0 + ddui::view.width, y + 0.5 * ITEM_HEIGHT);
            ddui::stroke();
            y += ITEM_HEIGHT;
            continue;
        }

        bool disabled = (item.action_index == 0 && item.sub_menu_index == -1);

        // Draw background selection color
        if (!disabled && selected_item_index == i) {
            ddui::begin_path();
            ddui::rect(0, y, ddui::view.width, ITEM_HEIGHT);
            ddui::fill_color(COLOR_BG_ACTIVE);
            ddui::fill();
        }

        // Draw text content
        ddui::font_face(FONT_FACE);
        ddui::font_size(FONT_SIZE);
        ddui::text_align(ddui::align::LEFT | ddui::align::BASELINE);
        ddui::fill_color(disabled ? COLOR_FG_DISABLED : selected_item_index == i ? COLOR_FG_ACTIVE : COLOR_FG);
        draw_text_in_box(x, y, w, ITEM_HEIGHT, item.text.c_str());

        // Draw sub-menu arrow
        if (item.sub_menu_index != -1) {
            ddui::sub_view(ddui::view.width - RIGHT_MARGIN_WIDTH, y, RIGHT_MARGIN_WIDTH, ITEM_HEIGHT);
            draw_arrow();
            ddui::restore();
        }
        
        // Draw check mark
        if (item.checked) {
            ddui::text_align(ddui::align::CENTER | ddui::align::MIDDLE);
            ddui::font_face("entypo");
            ddui::font_size(ITEM_HEIGHT);
            ddui::text(0.5 * LEFT_MARGIN_WIDTH, y + 0.5 * ITEM_HEIGHT, entypo::CHECK_MARK, NULL);
        }

        y += ITEM_HEIGHT;
    }
}

void DefaultMenuView::draw_arrow() {
    float size = 0.4 * ddui::view.height;
    float margin = 0.5 * (ddui::view.height - size);
    float x = ddui::view.width - margin - size;
    float y = margin;
    ddui::begin_path();
    ddui::move_to(x, y);
    ddui::line_to(x, y + size);
    ddui::line_to(x + size, y + 0.5 * size);
    ddui::close_path();
    ddui::fill();
}
