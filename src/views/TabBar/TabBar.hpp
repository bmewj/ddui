//
//  TabBar.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 27/06/2019.
//  Copyright © 2019 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_TabBar_hpp
#define ddui_TabBar_hpp

#include <vector>
#include <string>
#include <functional>
#include <ddui/core>
#include <ddui/views/Menu>
#include <ddui/views/PlainTextBox>

struct TabBar {

    enum ActionType {
        NO_ACTION,
        SWITCH_TO_TAB,
        NEW_TAB,
        CLOSE_TAB,
        RENAME_TAB,
        DUPLICATE_TAB,
        REORDER_TAB
    };

    struct Action {
        ActionType type;
        int tab_index; // used by CLOSE_TAB, RENAME_TAB, DUPLICATE_TAB, REORDER_TAB
        int reorder_to_index; // used by REORDER_TAB
        std::string new_name; // used by RENAME_TAB
    };

    enum UserInputState {
        IDLE,
        TAB_PRESSED,
        TAB_RENAMING
    };

    struct State {
        // Cached tab_names, active_tab
        std::vector<std::string> tab_names;
        int active_tab;

        // Private state
        UserInputState state = IDLE;
        std::vector<float> initial_tab_positions;
        float initial_tab_width;
        float initial_mx, initial_my;

        // Rename box
        TextEdit::Model text_box_model;
        PlainTextBox::State text_box_state;

        // Context menu state
        int ctx_menu_action = -1;
    };

    TabBar(State& state);
    TabBar& font(const char* font_face, float font_size);
    TabBar& hover_background_color(ddui::Color color_bg_hover);
    TabBar& text_color(ddui::Color color_text);
    TabBar& text_color_hover(ddui::Color color_text_hover);
    TabBar& text_color_active(ddui::Color color_text_active);
    TabBar& new_tab_button_placement(int placement);
    TabBar& render(const std::vector<std::string>& tab_names, int active_tab);
    void process_action(Action* action);

private:
    // Temporary state
    State& state;
    Action action;
    int new_tab_placement = ddui::align::RIGHT;
    float new_tab_button_width, new_tab_button_x, tab_width;
    float* tab_xs;

    // Styles
    const char* font_face         = "regular";
    float       font_size         = 20.0;
    ddui::Color color_bg_hover    = ddui::rgb(0xbbbbbb);
    ddui::Color color_text        = ddui::rgb(0x999999);
    ddui::Color color_text_hover  = ddui::rgb(0x888888);
    ddui::Color color_text_active = ddui::rgb(0x000000);

    void diff_state(const std::vector<std::string>& tab_names, int active_tab);
    void process_user_input();
    void calculate_tab_positions();
    void draw();

    void open_rename_input();
    void create_context_menu(MenuBuilder::Menu& menu);
    int tab_drag_target_position();
    void tab_drag_drop();

    static int find_removal(const std::vector<std::string>& a, const std::vector<std::string>& b);
    void render_button(float x, float width, const char* name, bool hovering, bool active);
};

#endif
