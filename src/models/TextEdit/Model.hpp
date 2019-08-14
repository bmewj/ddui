//
//  Model.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 19/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_TextEdit_Model_hpp
#define ddui_TextEdit_Model_hpp

#include <ddui/core>
#include <vector>
#include <memory>

namespace TextEdit {

struct Selection {
    int a_line, a_index;
    int b_line, b_index;
    int desired_index;
};

struct StyleCommand {

    enum Type {
        BOLD,
        SIZE,
        COLOR
    };

    Type type;
    union {
        bool bool_value;
        float float_value;
        ddui::Color color_value;
    };

};

struct Style {
    bool font_bold;
    float text_size;
    ddui::Color text_color;
};

struct Character {
    int index;
    int num_bytes;
    int entity_id;
    Style style;
};

struct Line {
    int num_bytes;
    std::unique_ptr<char[]> content;
    std::vector<Character> characters;
    Style style;
};

struct Model {
    Model();

    int version_count; // increments when state is changed
    std::vector<Line> lines;
    Selection selection;
    const char* regular_font;
    const char* bold_font;
};

void set_text_content(Model* model, const char* content);
void insert_text_content(Model* model, int* line, int* index, const char* content);
std::unique_ptr<char[]> get_text_content(Model* model, const Selection& selection);
std::unique_ptr<char[]> get_text_content(Model* model);

void set_style(Model* model, bool font_bold, float text_size, ddui::Color text_color);
void apply_style(Model* model, Selection selection, StyleCommand style);
void create_entity(Model* model, int line, int from, int to, int entity_id);
void apply_keyboard_input(Model* model, ddui::KeyState* key_state);

void delete_range(Model* model, const Selection& selection);
void insert_character(Model* model, int line, int index, const char* character);
void insert_line_break(Model* model, int line, int index);
void remove_line_breaks(Model* model);

}

#endif
