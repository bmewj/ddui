//
//  Menu.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 25/07/2019.
//  Copyright © 2019 Bartholomew Joyce All rights reserved.
//

#ifndef Menu_hpp
#define Menu_hpp

#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <ddui/core>
#include "MenuMouseTracker.hpp"

struct Menu {

    enum ActionType {
        NO_ACTION,
        MENU_DISMISS,
        ITEM_CLICK
    };

    struct Action {
        ActionType type;
        int menu_index;
        int item_index;
        int action_id; // -1 if no action id
    };

    struct ItemState {
        bool separator;
        bool checked;
        std::string text;
        int action_index; // 0 is disabled, negative is callback, positive is id
        int sub_menu_index; // -1 if no sub-menu
    };

    struct SubMenuState;
    struct BoundingRect;
    struct Anchor;

    struct IMenuView {
        virtual ~IMenuView() = default;
        virtual void lay_out_menu(float max_width, float max_height, BoundingRect*) = 0;
        virtual void get_item_anchors(const BoundingRect&, int item_index, Anchor*, Anchor*) = 0;
        virtual int  process_user_input(const BoundingRect&) = 0;
        virtual void render(const BoundingRect&, int selected_item_index) = 0;
    };

    struct SubMenuState {
        std::vector<ItemState> items;
        IMenuView* (*construct_view_state)(const SubMenuState& menu, int level);
    };

    struct BoundingRect {
        float x, y;
        float width, height;
    };

    struct Anchor {
        enum Direction {
            LEFT_TO_RIGHT,
            RIGHT_TO_LEFT
        };

        Direction direction;
        float x, y;
    };

    struct OpenedMenuState {
        int menu_index;
        int selected_item_index; // -1 is no selection
        Anchor anchor;
        BoundingRect bounding_rect;
        std::unique_ptr<IMenuView> view_state;
    };

    struct State {
        MenuMouseTracker::State mouse_tracker_state;
        std::vector<SubMenuState> menus;
        std::vector<std::function<void()>> action_callbacks;
        std::vector<int> action_ids;
        std::vector<OpenedMenuState> opened_menu_stack;
        float root_x, root_y, root_width;
    };

    Menu(State& state);
    ~Menu();
    Menu& process_user_input(Action* action = NULL);
    Menu& steal_user_input();
    Menu& render();

private:
    State& state;
    ddui::MouseState saved_mouse_state;
    bool did_lay_out = false;
    bool did_steal_user_input = false;

    void lay_out_menus();
    void lay_out_menu(const Anchor& a, const Anchor& b, OpenedMenuState& opened_menu);
    void choose_most_suitable_anchor(const Anchor& a, const Anchor& b, float width, Anchor* out);
};

#endif
