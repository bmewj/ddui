//
//  PlainTextBox.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 18/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_PlainTextBox_hpp
#define ddui_PlainTextBox_hpp

#include <ddui/models/TextEdit>
#include <vector>

struct PlainTextBox {

    using Model = TextEdit::Model;
    using Measurements = TextEdit::Measurements;

    struct State {
        // Text edit state
        int current_version_count = -1;
        Model* model = NULL;
        Measurements measurements;

        // UI info
        bool is_mouse_dragging = false;
        float height = 0;
        float scroll_x = 0;
    };

    // Styles
    struct StyleOptions {
        float margin;
        float border_radius;
        float border_width;
        ddui::Color border_color;
        ddui::Color border_color_focused;
        ddui::Color bg_color;
        ddui::Color bg_color_focused;
        ddui::Color cursor_color;
        ddui::Color selection_color;
        bool selection_in_foreground;
    };
    static StyleOptions* get_global_styles();

    // Methods
    PlainTextBox(State* state, Model* model);
    PlainTextBox& set_styles(const StyleOptions* styles);
    PlainTextBox& set_multiline(bool multiline);
    void update();

protected:
    State& state;
    Model& model;
    const StyleOptions* styles;
    bool multiline;

    virtual void process_key_input();
    virtual void refresh_model_measurements();
    virtual void draw_content();
    virtual void draw_selection();
};

#endif
