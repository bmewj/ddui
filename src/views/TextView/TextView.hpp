//
//  TextView.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 15/12/2024.
//  Copyright © 2024 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_TextView_hpp
#define ddui_TextView_hpp

#include <ddui/models/TextEdit>
#include <vector>

struct TextView {

    using Model = TextEdit::Model;
    using Measurements = TextEdit::Measurements;

    struct State {
        // Text edit state
        int current_version_count = -1;
        Model* model = NULL;
        Measurements measurements;

        // UI info
        bool is_mouse_dragging = false;
    };

    // Styles
    struct StyleOptions {
        float margin;
        ddui::Color cursor_color;
        ddui::Color selection_color;
        bool selection_in_foreground;
    };
    static StyleOptions* get_global_styles();

    // Methods
    TextView(State* state, Model* model);
    TextView& set_styles(const StyleOptions* styles);
    void update();

protected:
    State& state;
    Model& model;
    const StyleOptions* styles;

    virtual void process_key_input();
    virtual void refresh_model_measurements();
    virtual void measure_entity(int line, int index, int entity_id, float* width, float* height);
    virtual void draw_entity(int line, int index, int entity_id);
    virtual void draw_content();
    virtual void draw_selection(const ddui::Color& cursor_color);
};

#endif
