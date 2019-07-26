//
//  DefaultMenuView.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 25/07/2019.
//  Copyright © 2019 Bartholomew Joyce All rights reserved.
//

#ifndef Menu_DefaultMenuView_hpp
#define Menu_DefaultMenuView_hpp

#include "Menu.hpp"

struct DefaultMenuView : Menu::IMenuView {
    using SubMenuState = Menu::SubMenuState;
    using BoundingRect = Menu::BoundingRect;
    using Anchor = Menu::Anchor;

    // Implementation of IMenuView interface
    static Menu::IMenuView* construct();
    DefaultMenuView();
    void lay_out_menu(const SubMenuState&, float max_width, float max_height, BoundingRect*) override;
    void get_item_anchors(const SubMenuState&, const BoundingRect&, int item_index, Anchor*, Anchor*) override;
    int  process_user_input(const SubMenuState&, const BoundingRect&) override;
    void render(const SubMenuState&, const BoundingRect&, int selected_item_index) override;

    // Style options
    struct StyleOptions {
        float item_height;
        float border_radius;
        const char* font_face;
        float font_size;
        float left_margin_width;
        float right_margin_width;

        ddui::Color color_bg;
        ddui::Color color_fg;
        ddui::Color color_bg_active;
        ddui::Color color_fg_active;
        ddui::Color color_fg_disabled;
        
        float separator_width;
        ddui::Color separator_color;
    };
    static StyleOptions* get_global_styles();
    StyleOptions* styles;

    // Own methods
    void render_background(const SubMenuState&, const BoundingRect&, int selected_item_index);
    void render_content(const SubMenuState&, const BoundingRect&, int selected_item_index);

private:
    float content_width;
    float content_height;
    float scroll_y;

    void draw_items(const SubMenuState&, int selected_item_index);
    void draw_arrow();
};

#endif
