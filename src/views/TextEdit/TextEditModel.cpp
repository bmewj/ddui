//
//  TextEditModel.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 12/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "TextEditModel.hpp"
#include <cstdlib>
#include <string.h>

namespace TextEdit {

void set_text_content(TextEditModel* model, const char* content) {
    
    // Reset the model
    model->lines.clear();
    model->selection = { 0 };
    
    // Copy over current
    const char* ptr = content;
    const char* line_start = content;
    TextEditModel::Line current_line;
    
    while (*ptr != '\0') {
        if (*ptr == '\n') {
            auto content = new char[ptr - line_start + 1];
            strncpy(content, line_start, ptr - line_start);
            content[ptr - line_start] = '\0';
            current_line.num_bytes = ptr - line_start + 1;
            current_line.content = std::unique_ptr<char[]>(content);
        
            model->lines.push_back(std::move(current_line));
            line_start = ++ptr;
            continue;
        }
        
        TextEditModel::Character character;
        character.index = ptr - line_start;
        character.num_bytes = (
            !(*ptr & 0x80) ? 1 :
            !(*ptr & 0x20) ? 2 :
            !(*ptr & 0x10) ? 3 : 4
        );
        character.entity_id = -1;
        character.text_size = 48.0;
        character.font_bold = false;
        character.text_color = nvgRGB(0, 0, 0);
        ptr += character.num_bytes;
        
        current_line.characters.push_back(std::move(character));
    }
    
    {
        auto content = new char[ptr - line_start + 1];
        strncpy(content, line_start, ptr - line_start);
        content[ptr - line_start] = '\0';
        current_line.num_bytes = ptr - line_start + 1;
        current_line.content = std::unique_ptr<char[]>(content);
    
        model->lines.push_back(std::move(current_line));
    }
    
    // Increment model version
    model->version_count++;
    
}

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)

static void apply_style_to_line(TextEditModel::Line* line, int a_index, int b_index, TextEditModel::StyleCommand style);

void apply_style(TextEditModel* model, TextEditModel::Selection selection, TextEditModel::StyleCommand style) {

    // Single-line selection
    if (selection.a_line == selection.b_line) {
        if (selection.a_line >= 0 && selection.a_line < model->lines.size()) {
            apply_style_to_line(&model->lines[selection.a_line], selection.a_index, selection.b_index, style);
        }
        return;
    }
    
    // Multi-line selection
    int min_line = selection.a_line < selection.b_line ? selection.a_line : selection.b_line;
    int max_line = selection.a_line > selection.b_line ? selection.a_line : selection.b_line;
    int min_i = selection.a_line < selection.b_line ? selection.a_index : selection.b_index;
    int max_i = selection.a_line < selection.b_line ? selection.a_index : selection.b_index;
    
    if (min_line >= 0 && min_line < model->lines.size()) {
        int min_i_b = model->lines[min_line].characters.size();
        apply_style_to_line(&model->lines[min_line], min_i, min_i_b, style);
    }
    for (int line = min_line + 1; line < max_line; ++line) {
        if (line >= 0 && line < model->lines.size()) {
            int b_index = model->lines[line].characters.size();
            apply_style_to_line(&model->lines[line], 0, b_index, style);
        }
    }
    if (max_line >= 0 && max_line < model->lines.size()) {
        apply_style_to_line(&model->lines[max_line], 0, max_i, style);
    }

    model->version_count++;
    
}

void apply_style_to_line(TextEditModel::Line* line, int a_index, int b_index, TextEditModel::StyleCommand style) {
    int min_i = MAX(MIN(a_index, b_index), 0);
    int max_i = MIN(MAX(a_index, b_index), line->characters.size());

    for (int i = min_i; i < max_i; ++i) {
        switch (style.type) {
            case TextEditModel::StyleCommand::SIZE:
                line->characters[i].text_size = style.float_value;
                break;
            case TextEditModel::StyleCommand::BOLD:
                line->characters[i].font_bold = style.bool_value;
                break;
            case TextEditModel::StyleCommand::COLOR:
                line->characters[i].text_color = style.color_value;
                break;
        }
    }
}

void create_entity(TextEditModel* model, int lineno, int from, int to, int entity_id) {
    if (from > to) {
        throw "Entity range must be greater than 1.";
    }
    if (!(lineno >= 0 && lineno < model->lines.size())) {
        printf("TextEdit::TextEditModel::create_entity() on line that doesn't exist.");
        return; // Invalid line ranges are not thrown.
    }

    auto& line = model->lines[lineno];
    if (!(from >= 0 && from <  line.characters.size() &&
            to >  0 &&   to <= line.characters.size())) {
        printf("TextEdit::TextEditModel::create_entity() on index range that doesn't exist.");
        return; // Invalid index ranges are not thrown.
    }
    
    auto& first_character = line.characters[from];
    auto& last_character = line.characters[to - 1];
    first_character.num_bytes = last_character.index + last_character.num_bytes - first_character.index;
    first_character.entity_id = entity_id;
    
    auto begin_iter = line.characters.begin() + from + 1;
    auto end_iter = line.characters.begin() + to;
    line.characters.erase(begin_iter, end_iter);
}

}
