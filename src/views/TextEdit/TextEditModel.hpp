//
//  TextEditModel.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 12/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_TextEditModel_hpp
#define ddui_TextEditModel_hpp

#include <wchar.h>
#include <nanovg.h>
#include <vector>
#include <memory>

namespace TextEdit {

struct TextEditModel {

    struct Selection {
        int a_line, a_index;
        int b_line, b_index;
    };

    struct StyleCommand {

        enum Type {
            SIZE,
            BOLD,
            COLOR
        };

        Type type;
        union {
            bool bool_value;
            float float_value;
            NVGcolor color_value;
        };

    };

    struct Character {
        int index;
        int num_bytes;
        int entity_id;

        bool font_bold;
        float text_size;
        NVGcolor text_color;
    };

    struct Line {
        int num_bytes;
        std::unique_ptr<char[]> content;
        std::vector<Character> characters;
    };

    int version_count; // increments when state is changed
    std::vector<Line> lines;
    Selection selection;

};

void set_text_content(TextEditModel* model, const char* content);
void apply_style(TextEditModel* model, TextEditModel::Selection selection, TextEditModel::StyleCommand style);
void create_entity(TextEditModel* model, int line, int from, int to, int entity_id);

}

#endif
