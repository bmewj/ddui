//
//  MenuBuilder.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 25/07/2019.
//  Copyright © 2019 Bartholomew Joyce All rights reserved.
//

#ifndef MenuBuilder_hpp
#define MenuBuilder_hpp

#include "Menu.hpp"
#include "DefaultMenuView.hpp"

struct MenuBuilder {
    struct Menu {
        inline Menu(MenuBuilder& builder);
        inline Menu& separator();
        inline Menu& item(const std::string& text);
        inline Menu& checked(bool checked = true);
        inline Menu& action(int action_id);
        inline Menu& action(std::function<void()> action_callback);
        inline Menu& sub_menu(Menu& sub_menu);
        inline Menu& custom_view(std::function<::Menu::IMenuView*()>&& construct_view_state);

        MenuBuilder& builder;
        int menu_index;

    private:
        friend struct MenuBuilder;
    };

    Menu menu() {
        return Menu(*this);
    }

    ::Menu::State create(Menu& root_menu, float x, float y, float width = 0) {
        auto& root = state.menus[root_menu.menu_index];

        state.opened_menu_stack.push_back(::Menu::OpenedMenuState());
        auto& opened_menu = state.opened_menu_stack.back();
        opened_menu.menu_index = root_menu.menu_index;
        opened_menu.selected_item_index = -1;
        opened_menu.view_state = std::unique_ptr<::Menu::IMenuView>(root.construct_view_state());

        state.root_x = x;
        state.root_y = y;
        state.root_width = width;

        return std::move(state);
    }

private:
    ::Menu::State state;
};

MenuBuilder::Menu::Menu(MenuBuilder& builder) : builder(builder) {
    menu_index = builder.state.menus.size();
    builder.state.menus.push_back(::Menu::SubMenuState());
    auto& sub_menu = builder.state.menus.back();
    sub_menu.construct_view_state = DefaultMenuView::construct;
}

MenuBuilder::Menu& MenuBuilder::Menu::separator() {
    auto& menu = builder.state.menus[menu_index];
    menu.items.push_back(::Menu::ItemState());

    auto& item = menu.items.back();
    item.separator = true;
    item.checked = false;
    item.action_index = 0;
    item.sub_menu_index = -1;

    return *this;
}

MenuBuilder::Menu& MenuBuilder::Menu::item(const std::string& text) {
    auto& menu = builder.state.menus[menu_index];
    menu.items.push_back(::Menu::ItemState());

    auto& item = menu.items.back();
    item.separator = false;
    item.checked = false;
    item.text = text;
    item.action_index = 0;
    item.sub_menu_index = -1;

    return *this;
}

MenuBuilder::Menu& MenuBuilder::Menu::checked(bool checked) {
    auto& menu = builder.state.menus[menu_index];
    auto& item = menu.items.back();
    item.checked = checked;
    return *this;
}

MenuBuilder::Menu& MenuBuilder::Menu::action(int action_id) {
    if (action_id == -1) {
        return *this;
    }

    auto& menu = builder.state.menus[menu_index];
    auto& item = menu.items.back();
    builder.state.action_ids.push_back(action_id);
    item.action_index = builder.state.action_ids.size();
    return *this;
}

MenuBuilder::Menu& MenuBuilder::Menu::action(std::function<void()> action_callback) {
    auto& menu = builder.state.menus[menu_index];
    auto& item = menu.items.back();
    builder.state.action_callbacks.push_back(std::move(action_callback));
    item.action_index = - builder.state.action_callbacks.size();
    return *this;
}

MenuBuilder::Menu& MenuBuilder::Menu::sub_menu(MenuBuilder::Menu& sub_menu) {
    auto& menu = builder.state.menus[menu_index];
    auto& item = menu.items.back();
    item.sub_menu_index = sub_menu.menu_index;
    return *this;
}

MenuBuilder::Menu& MenuBuilder::Menu::custom_view(std::function<::Menu::IMenuView*()>&& construct_view_state) {
    auto& menu = builder.state.menus[menu_index];
    menu.construct_view_state = std::move(construct_view_state);
    return *this;
}

#endif
