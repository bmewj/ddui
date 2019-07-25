//
//  SubMenuView.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 25/07/2019.
//  Copyright © 2019 Bartholomew Joyce All rights reserved.
//

#ifndef Menu_SubMenuView_hpp
#define Menu_SubMenuView_hpp

#include "Menu.hpp"

struct SubMenuView : Menu::ISubMenuView {
    using SubMenuState = Menu::SubMenuState;
    using BoundingRect = Menu::BoundingRect;
    using Anchor = Menu::Anchor;

    static Menu::ISubMenuView* construct();
    void get_bounding_rect(const SubMenuState&, float max_width, float max_height, BoundingRect*) override;
    void get_item_anchors(const SubMenuState&, const BoundingRect&, int item_index, Anchor*, Anchor*) override;
    int  process_user_input(const SubMenuState&, const BoundingRect&) override;
    void render(const SubMenuState&, const BoundingRect&, int selected_item_index) override;

private:
    float content_width;
    float content_height;
    float scroll_y;

    void draw_items(const SubMenuState&, int selected_item_index);
    void draw_arrow();
};

#endif
