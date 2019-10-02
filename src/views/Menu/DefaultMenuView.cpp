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

Menu::IMenuView* DefaultMenuView::construct() {
    return (Menu::IMenuView*)new DefaultMenuView();
}

DefaultMenuView::DefaultMenuView() {
    styles = get_global_styles();
    scroll_y = 0;
}

void DefaultMenuView::lay_out_menu(const SubMenuState& menu, int level, float max_width, float max_height, BoundingRect* rect) {

    float width;
    {
        ddui::font_face(styles->font_face);
        ddui::font_size(styles->font_size);
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

        width = styles->left_margin_width + max_text_width + styles->right_margin_width;
    }

    float height;
    {
        height = menu.items.size() * styles->item_height + 2 * styles->border_radius;
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
    rect->y = styles->border_radius; // (*)
    rect->width = width;
    rect->height = height;

    // (*) The x, y coordinates in the bounds object signify the x, y offset
    // of the first item in the menu. This is used to properly align the menu
    // when it is first opened such that the selected item in the parent menu
    // is lined up with the first item of this menu.
}

void DefaultMenuView::get_item_anchors(const SubMenuState& menu, int level, const BoundingRect& rect, int item_index, Anchor* a, Anchor* b) {
    float item_y = rect.y - scroll_y + styles->border_radius + item_index * styles->item_height;
    
    a->direction = Anchor::LEFT_TO_RIGHT;
    a->x = rect.x + rect.width;
    a->y = item_y;

    b->direction = Anchor::RIGHT_TO_LEFT;
    b->x = rect.x;
    b->y = item_y;
}

int DefaultMenuView::process_user_input(const SubMenuState& menu, int level, const BoundingRect& rect) {
    float first_item_y = rect.y - scroll_y + styles->border_radius;

    float mx, my;
    ddui::mouse_position(&mx, &my);

    auto index = (int)((my - first_item_y) / styles->item_height);
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

void DefaultMenuView::render(const SubMenuState& menu, int level, const BoundingRect& rect, int selected_item_index) {
    render_background(rect, selected_item_index);
    render_content(menu, rect, selected_item_index);
}

DefaultMenuView::StyleOptions* DefaultMenuView::get_global_styles() {
    static StyleOptions* styles = NULL;
    if (styles) {
        return styles;
    }

    styles = new StyleOptions;

    styles->item_height = 20.0;
    styles->border_radius = 4.0;
    styles->font_face = "regular";
    styles->font_size = 16.0;
    styles->left_margin_width = 32.0;
    styles->right_margin_width = 28.0;

    styles->color_bg = ddui::rgb(0xffffff);
    styles->color_fg = ddui::rgb(0x000000);
    styles->color_bg_active = ddui::rgb(0x5d92f1);
    styles->color_fg_active = ddui::rgb(0xffffff);
    styles->color_fg_disabled = ddui::rgb(0xbbbbbb);
    
    styles->separator_width = 1.0;
    styles->separator_color = ddui::rgb(0xdddddd);

    return styles;
}

void DefaultMenuView::render_background(const BoundingRect& rect, int selected_item_index) {
    // Menu shadow
    ddui::sub_view(rect.x, rect.y, rect.width, rect.height);
    {
        ddui::BoxShadow()
            .blur(1.5)
            .border_radius(styles->border_radius)
            .offset(0.5, 0.5)
            .render();
    }
    ddui::restore();

    // Menu background
    ddui::begin_path();
    ddui::rounded_rect(rect.x, rect.y, rect.width, rect.height, styles->border_radius);
    ddui::fill_color(styles->color_bg);
    ddui::fill();
}

void DefaultMenuView::render_content(const SubMenuState& menu, const BoundingRect& rect, int selected_item_index) {
    ddui::save(); // a
    ddui::clip(rect.x, rect.y, rect.width, rect.height);
    ddui::translate(0, -scroll_y);

    ddui::sub_view(rect.x, rect.y + styles->border_radius, rect.width, menu.items.size() * styles->item_height); // b

    draw_items(menu, selected_item_index);

    ddui::restore(); // b
    ddui::restore(); // a
}

void DefaultMenuView::draw_items(const SubMenuState& menu, int selected_item_index) {
    float x = styles->left_margin_width;
    float y = 0;
    float w = ddui::view.width - styles->left_margin_width - styles->right_margin_width + 5;
    
    for (int i = 0; i < menu.items.size(); ++i) {
        const auto& item = menu.items[i];

        // Draw separator
        if (item.separator) {
            ddui::begin_path();
            ddui::stroke_color(styles->separator_color);
            ddui::stroke_width(styles->separator_width);
            ddui::move_to(0, y + 0.5 * styles->item_height);
            ddui::line_to(0 + ddui::view.width, y + 0.5 * styles->item_height);
            ddui::stroke();
            y += styles->item_height;
            continue;
        }

        bool disabled = (item.action_index == 0 && item.sub_menu_index == -1);

        // Draw background selection color
        if (!disabled && selected_item_index == i) {
            ddui::begin_path();
            ddui::rect(0, y, ddui::view.width, styles->item_height);
            ddui::fill_color(styles->color_bg_active);
            ddui::fill();
        }

        // Draw text content
        ddui::font_face(styles->font_face);
        ddui::font_size(styles->font_size);
        ddui::text_align(ddui::align::LEFT | ddui::align::BASELINE);
        ddui::fill_color(
            disabled ? styles->color_fg_disabled :
            selected_item_index == i ? styles->color_fg_active :
            styles->color_fg);
        draw_text_in_box(x, y, w, styles->item_height, item.text.c_str());

        // Draw sub-menu arrow
        if (item.sub_menu_index != -1) {
            ddui::sub_view(ddui::view.width - styles->right_margin_width, y, styles->right_margin_width, styles->item_height);
            draw_arrow();
            ddui::restore();
        }
        
        // Draw check mark
        if (item.checked) {
            ddui::text_align(ddui::align::CENTER | ddui::align::MIDDLE);
            ddui::font_face("entypo");
            ddui::font_size(styles->item_height);
            ddui::text(0.5 * styles->left_margin_width, y + 0.5 * styles->item_height, entypo::CHECK_MARK, NULL);
        }

        y += styles->item_height;
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
