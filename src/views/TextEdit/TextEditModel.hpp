//
//  TextEditModel.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 12/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_TextEditModel_hpp
#define ddui_TextEditModel_hpp

#include <nanovg.h>
#include <ddui/Context>
#include <vector>
#include <memory>

namespace TextEdit {

struct TextEditModel {

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
            NVGcolor color_value;
        };

    };

    struct Style {
        bool font_bold;
        float text_size;
        NVGcolor text_color;
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

    int version_count; // increments when state is changed
    std::vector<Line> lines;
    Selection selection;

};

void set_text_content(TextEditModel* model, const char* content);
void insert_text_content(TextEditModel* model, int* line, int* index, const char* content);
char* get_text_content(TextEditModel* model, TextEditModel::Selection selection);

void apply_style(TextEditModel* model, TextEditModel::Selection selection, TextEditModel::StyleCommand style);
void create_entity(TextEditModel* model, int line, int from, int to, int entity_id);
void apply_keyboard_input(TextEditModel* model, KeyState* key_state);

void delete_range(TextEditModel* model, TextEditModel::Selection selection);
void insert_character(TextEditModel* model, int line, int index, const char* character);
void insert_line_break(TextEditModel* model, int line, int index);

}

#endif
