//
//  TextEdit.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 19/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "Model.hpp"
#include <ddui/core>
#include <cstdlib>
#include <algorithm>
#include <string.h>
#include <stdio.h>

namespace TextEdit {

using namespace ddui;

Model::Model() {
    auto empty_content = new char[1];
    empty_content[0] = '\0';

    lines.push_back(Line());
    auto& empty_line = lines.front();
    empty_line.num_bytes = 1;
    empty_line.content = std::unique_ptr<char[]>(empty_content);
    empty_line.style.font_bold = false;
    empty_line.style.text_size = 16.0;
    empty_line.style.text_color = rgb(0x000000);

    version_count = 0;
    selection = { 0 };

    regular_font = "regular";
    bold_font = "bold";
}

void set_text_content(Model* model, const char* content) {
    
    // Get default styles
    auto default_style = model->lines.front().style;
    
    // Reset the model
    model->lines.clear();
    model->selection = { 0 };
    
    // Copy over current
    const char* ptr = content;
    const char* line_start = content;
    Line current_line;
    
    while (*ptr != '\0') {
        if (*ptr == '\n') {
            auto content = new char[ptr - line_start + 1];
            strncpy(content, line_start, ptr - line_start);
            content[ptr - line_start] = '\0';
            current_line.num_bytes = int(ptr - line_start) + 1;
            current_line.content = std::unique_ptr<char[]>(content);
            current_line.style = default_style;
        
            model->lines.push_back(std::move(current_line));
            line_start = ++ptr;
            continue;
        }

        Character character;
        character.index = int(ptr - line_start);
        character.num_bytes = (
            !(*ptr & 0x80) ? 1 :
            !(*ptr & 0x20) ? 2 :
            !(*ptr & 0x10) ? 3 : 4
        );
        character.entity_id = -1;
        character.style = default_style;
        ptr += character.num_bytes;
        
        current_line.characters.push_back(std::move(character));
    }
    
    {
        auto content = new char[ptr - line_start + 1];
        strncpy(content, line_start, ptr - line_start);
        content[ptr - line_start] = '\0';
        current_line.num_bytes = int(ptr - line_start) + 1;
        current_line.content = std::unique_ptr<char[]>(content);
        current_line.style = default_style;
    
        model->lines.push_back(std::move(current_line));
    }
    
    // Increment model version
    model->version_count++;
    
}

void insert_text_content(Model* model, int* lineno, int* index, const char* content) {
    
    std::vector<Line> lines;
    
    // Copy over current
    const char* ptr = content;
    const char* line_start = content;
    Line current_line;
    
    // Get basic styles
    Style style;
    if (*index == 0) {
        style = model->lines[*lineno].style;
    } else {
        style = model->lines[*lineno].characters[*index - 1].style;
    }
    
    // Prepare all the lines
    while (*ptr != '\0') {
        if (*ptr == '\n') {
            auto content = new char[ptr - line_start + 1];
            strncpy(content, line_start, ptr - line_start);
            content[ptr - line_start] = '\0';
            current_line.num_bytes = int(ptr - line_start) + 1;
            current_line.content = std::unique_ptr<char[]>(content);
            current_line.style = style;
        
            lines.push_back(std::move(current_line));
            line_start = ++ptr;
            continue;
        }
        
        Character character;
        character.index = int(ptr - line_start);
        character.num_bytes = (
            !(*ptr & 0x80) ? 1 :
            !(*ptr & 0x20) ? 2 :
            !(*ptr & 0x10) ? 3 : 4
        );
        character.entity_id = -1;
        character.style = style;
        ptr += character.num_bytes;
        
        current_line.characters.push_back(std::move(character));
    }
    
    {
        auto content = new char[ptr - line_start + 1];
        strncpy(content, line_start, ptr - line_start);
        content[ptr - line_start] = '\0';
        current_line.num_bytes = int(ptr - line_start) + 1;
        current_line.content = std::unique_ptr<char[]>(content);
        current_line.style = style;
    
        lines.push_back(std::move(current_line));
    }
    
    // Now we insert the lines into the model
    if (lines.size() == 1) {
        auto& line = model->lines[*lineno];
        auto& child_line = lines.front();
        
        int ch_index = *index == 0 ? 0 : line.characters[*index - 1].index + line.characters[*index - 1].num_bytes;
        
        auto new_num_bytes = line.num_bytes + child_line.num_bytes - 1;
        auto new_content = new char[new_num_bytes];
        strncpy(new_content, line.content.get(), ch_index);
        strncpy(new_content + ch_index, child_line.content.get(), child_line.num_bytes - 1);
        strncpy(new_content + ch_index + child_line.num_bytes - 1, line.content.get() + ch_index, line.num_bytes - ch_index);
        line.content = std::unique_ptr<char[]>(new_content);
        line.num_bytes = new_num_bytes;
        
        for (auto& character : child_line.characters) {
            character.index += ch_index;
        }
        for (auto it = line.characters.begin() + *index; it < line.characters.end(); ++it) {
            it->index += child_line.num_bytes - 1;
        }
        
        line.characters.insert(line.characters.begin() + *index, child_line.characters.begin(), child_line.characters.end());
        
        *index += child_line.characters.size();

    } else {
        auto& line = model->lines[*lineno];
        auto& child_a_line = lines.front();
        auto& child_b_line = lines.back();
        
        auto ch_index = *index == 0 ? 0 : line.characters[*index - 1].index + line.characters[*index - 1].num_bytes;
        auto b_ch_index = child_b_line.characters.empty() ? 0 : child_b_line.characters.back().index +
                                                                child_b_line.characters.back().num_bytes;
        auto b_size = child_b_line.characters.size();
        
        auto a_num_bytes = ch_index + child_a_line.num_bytes;
        auto a_content = new char[a_num_bytes];
        strncpy(a_content, line.content.get(), ch_index);
        strncpy(a_content + ch_index, child_a_line.content.get(), child_a_line.num_bytes);
        
        auto b_num_bytes = child_b_line.num_bytes + line.num_bytes - ch_index;
        auto b_content = new char[b_num_bytes];
        strncpy(b_content, child_b_line.content.get(), child_b_line.num_bytes - 1);
        strncpy(b_content + child_b_line.num_bytes - 1, line.content.get() + ch_index, line.num_bytes - ch_index);
        
        child_a_line.content = std::unique_ptr<char[]>(a_content);
        child_a_line.num_bytes = a_num_bytes;
        for (int i = 0; i < *index; ++i) {
            child_a_line.characters.insert(child_a_line.characters.begin() + i, line.characters[i]);
        }
        for (int i = *index; i < child_a_line.characters.size(); ++i) {
            child_a_line.characters[i].index += ch_index;
        }
        
        child_b_line.content = std::unique_ptr<char[]>(b_content);
        child_b_line.num_bytes = b_num_bytes;
        for (int i = *index; i < line.characters.size(); ++i) {
            line.characters[i].index += b_ch_index - ch_index;
            child_b_line.characters.push_back(line.characters[i]);
        }
        
        model->lines.erase(model->lines.begin() + *lineno);
        for (int i = 0; i < lines.size(); ++i) {
            model->lines.insert(model->lines.begin() + *lineno + i, std::move(lines[i]));
        }
        
        *lineno += lines.size() - 1;
        *index = int(b_size);
    }
    
    // Increment model version
    model->version_count++;
    
}

std::unique_ptr<char[]> get_text_content(const Model* model, const Selection& selection) {
    
    if (selection.a_line == selection.b_line) {
        // Single-line copy string
        auto& line = model->lines[selection.a_line];
        
        auto from_index = selection.a_index < selection.b_index ? selection.a_index : selection.b_index;
        auto to_index   = selection.a_index > selection.b_index ? selection.a_index : selection.b_index;
        
        auto ch_from_index = from_index == 0 ? 0 : line.characters[from_index - 1].index +
                                                   line.characters[from_index - 1].num_bytes;
        auto ch_to_index   = to_index   == 0 ? 0 : line.characters[to_index - 1].index +
                                                   line.characters[to_index - 1].num_bytes;
        
        auto num_bytes = ch_to_index - ch_from_index + 1;
        auto text_content = new char[num_bytes];
        strncpy(text_content, line.content.get() + ch_from_index, num_bytes - 1);
        text_content[num_bytes - 1] = '\0';

        return std::unique_ptr<char[]>(text_content);
        
    } else {
        // Multi-line copy string
        
        auto  from_lineno   = selection.a_line < selection.b_line ? selection.a_line : selection.b_line;
        auto  from_index    = selection.a_line < selection.b_line ? selection.a_index : selection.b_index;
        auto& from_line     = model->lines[from_lineno];
        auto  from_ch_index = from_index == 0 ? 0 : from_line.characters[from_index - 1].index +
                                                    from_line.characters[from_index - 1].num_bytes;
        
        auto  to_lineno   = selection.a_line > selection.b_line ? selection.a_line : selection.b_line;
        auto  to_index    = selection.a_line > selection.b_line ? selection.a_index : selection.b_index;
        auto& to_line     = model->lines[to_lineno];
        auto  to_ch_index = to_index == 0 ? 0 : to_line.characters[to_index - 1].index +
                                                to_line.characters[to_index - 1].num_bytes;
        
        // Count text length
        int num_bytes = 0;
        {
            num_bytes += from_line.num_bytes - from_ch_index;
        }
        for (int i = from_lineno + 1; i < to_lineno; ++i) {
            num_bytes += model->lines[i].num_bytes;
        }
        {
            num_bytes += to_ch_index + 1;
        }
        
        // Copy data over
        auto text_content = new char[num_bytes];
        auto ptr = text_content;
        {
            strncpy(ptr, from_line.content.get() + from_ch_index, from_line.num_bytes - from_ch_index - 1);
            ptr[from_line.num_bytes - from_ch_index - 1] = '\n';
            ptr += from_line.num_bytes - from_ch_index;
        }
        for (int i = from_lineno + 1; i < to_lineno; ++i) {
            auto& line = model->lines[i];
            strncpy(ptr, line.content.get(), line.num_bytes - 1);
            ptr[line.num_bytes - 1] = '\n';
            ptr += line.num_bytes;
        }
        {
            strncpy(ptr, to_line.content.get(), to_ch_index);
            ptr[to_ch_index] = '\0';
            ptr += to_ch_index + 1;
        }
        
        return std::unique_ptr<char[]>(text_content);
    }
}

std::unique_ptr<char[]> get_text_content(const Model* model) {

    // Select all
    Selection selection;
    selection.a_line = 0;
    selection.a_index = 0;
    selection.b_line = int(model->lines.size()) - 1;
    selection.b_index = int(model->lines.back().characters.size());

    // Return the text content
    return get_text_content(model, selection);

}

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)

static void apply_style_to_line(Line* line, int a_index, int b_index, StyleCommand style);

void set_style(Model* model, bool font_bold, float text_size, Color text_color) {

    Selection selection = { 0 };
    selection.b_line = int(model->lines.size());

    StyleCommand command;
    
    command.type = StyleCommand::BOLD;
    command.bool_value = font_bold;
    apply_style(model, selection, command);
    
    command.type = StyleCommand::SIZE;
    command.float_value = text_size;
    apply_style(model, selection, command);
    
    command.type = StyleCommand::COLOR;
    command.color_value = text_color;
    apply_style(model, selection, command);
    
}

void apply_style(Model* model, Selection selection, StyleCommand style) {

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
        int min_i_b = int(model->lines[min_line].characters.size());
        apply_style_to_line(&model->lines[min_line], min_i, min_i_b, style);
    }
    for (int line = min_line + 1; line < max_line; ++line) {
        if (line >= 0 && line < model->lines.size()) {
            int b_index = int(model->lines[line].characters.size());
            apply_style_to_line(&model->lines[line], 0, b_index, style);
        }
    }
    if (max_line >= 0 && max_line < model->lines.size()) {
        apply_style_to_line(&model->lines[max_line], 0, max_i, style);
    }

    model->version_count++;
    
}

void apply_style_to_line(Line* line, int a_index, int b_index, StyleCommand style) {
    int min_i = MAX(MIN(a_index, b_index), 0);
    int max_i = MIN(MAX(a_index, b_index), int(line->characters.size()));
    
    if (min_i == 0) {
        switch (style.type) {
            case StyleCommand::BOLD:
                line->style.font_bold  = style.bool_value;
                break;
            case StyleCommand::SIZE:
                line->style.text_size  = style.float_value;
                break;
            case StyleCommand::COLOR:
                line->style.text_color = style.color_value;
                break;
        }
    }

    for (int i = min_i; i < max_i; ++i) {
        switch (style.type) {
            case StyleCommand::BOLD:
                line->characters[i].style.font_bold  = style.bool_value;
                break;
            case StyleCommand::SIZE:
                line->characters[i].style.text_size  = style.float_value;
                break;
            case StyleCommand::COLOR:
                line->characters[i].style.text_color = style.color_value;
                break;
        }
    }
}

void create_entity(Model* model, int lineno, int from, int to, int entity_id) {
    if (from > to) {
        throw "Entity range must be greater than 1.";
    }
    if (!(lineno >= 0 && lineno < model->lines.size())) {
        printf("TextEdit::create_entity() on line that doesn't exist.");
        return; // Invalid line ranges are not thrown.
    }

    auto& line = model->lines[lineno];
    if (!(from >= 0 && from <  line.characters.size() &&
            to >  0 &&   to <= line.characters.size())) {
        printf("TextEdit::create_entity() on index range that doesn't exist.");
        return; // Invalid index ranges are not thrown.
    }

    // Update the num_bytes and entity_id of the first character in the range
    auto& first_character = line.characters[from];
    auto& last_character = line.characters[to - 1];
    first_character.num_bytes = last_character.index + last_character.num_bytes - first_character.index;
    first_character.entity_id = entity_id;

    // Delete all subsequent characters
    auto begin_iter = line.characters.begin() + from + 1;
    auto end_iter = line.characters.begin() + to;
    line.characters.erase(begin_iter, end_iter);

    // Update the selection (if necessary)
    auto& sel = model->selection;
    if (sel.a_line == lineno && sel.a_index >= from) {
        sel.a_index = (sel.a_index < to) ? from + 1 : (sel.a_index + from - to + 1);
    }
    if (sel.b_line == lineno && sel.b_index >= from) {
        sel.b_index = (sel.b_index < to) ? from + 1 : (sel.b_index + from - to + 1);
    }

    model->version_count++;
}

void apply_keyboard_input(Model* model, KeyState* key_state, bool readonly) {

    if (key_state->action != keyboard::ACTION_PRESS && key_state->action != keyboard::ACTION_REPEAT) {
        return;
    }

    bool range_is_selected = (model->selection.a_line != model->selection.b_line ||
                              model->selection.a_index != model->selection.b_index);

    if (key_state->key == keyboard::KEY_LEFT) {
        auto& sel = model->selection;
        
        if (range_is_selected && !(key_state->mods & (keyboard::MOD_COMMAND | keyboard::MOD_SHIFT))) {
            if (sel.b_line > sel.a_line || (sel.b_line == sel.a_line && sel.b_index > sel.a_index)) {
                sel.b_line = sel.a_line;
                sel.b_index = sel.a_index;
            }
        } else if (key_state->mods & keyboard::MOD_COMMAND) {
            sel.b_index = 0;
        } else if (sel.b_index - 1 >= 0) {
            sel.b_index -= 1;;
        } else if (sel.b_line - 1 >= 0) {
            sel.b_index = int(model->lines[sel.b_line - 1].characters.size());
            sel.b_line -= 1;
        }
        
        if (!(key_state->mods & keyboard::MOD_SHIFT)) {
            sel.a_index = sel.b_index;
            sel.a_line = sel.b_line;
        }
        
        sel.desired_index = sel.b_index;
        
        *key_state = { 0 };
    }

    if (key_state->key == keyboard::KEY_HOME) {
        auto& sel = model->selection;
        sel.desired_index = sel.b_index = 0;
        if (!(key_state->mods & keyboard::MOD_SHIFT)) {
            sel.a_index = 0;
            sel.a_line = sel.b_line;
        }
        *key_state = { 0 };
    }
    
    if (key_state->key == keyboard::KEY_END) {
        auto& sel = model->selection;
        auto line_length = int(model->lines[sel.b_line].characters.size());
        sel.desired_index = sel.b_index = line_length;
        if (!(key_state->mods & keyboard::MOD_SHIFT)) {
            sel.a_index = line_length;
            sel.a_line = sel.b_line;
        }
        *key_state = { 0 };
    }
    
    if (key_state->key == keyboard::KEY_RIGHT) {
        auto& sel = model->selection;
        auto line_length = int(model->lines[sel.b_line].characters.size());
        
        if (range_is_selected && !(key_state->mods & (keyboard::MOD_COMMAND | keyboard::MOD_SHIFT))) {
            if (sel.b_line < sel.a_line || (sel.b_line == sel.a_line && sel.b_index < sel.a_index)) {
                sel.b_line = sel.a_line;
                sel.b_index = sel.a_index;
            }
        } else if (key_state->mods & keyboard::MOD_COMMAND) {
            sel.b_index = line_length;
        } else if (sel.b_index + 1 <= line_length) {
            sel.b_index += 1;
        } else if (sel.b_line + 1 < model->lines.size()) {
            sel.b_index = 0;
            sel.b_line += 1;
        }
        
        if (!(key_state->mods & keyboard::MOD_SHIFT)) {
            sel.a_index = sel.b_index;
            sel.a_line = sel.b_line;
        }
        
        sel.desired_index = sel.b_index;
        
        *key_state = { 0 };
    }

    if (key_state->key == keyboard::KEY_UP && (key_state->mods & keyboard::MOD_COMMAND)) {
        auto& sel = model->selection;

        sel.b_index = 0;
        sel.b_line = 0;

        if (!(key_state->mods & keyboard::MOD_SHIFT)) {
            sel.a_index = sel.b_index;
            sel.a_line = sel.b_line;
        }

        *key_state = { 0 };
    }

    if (key_state->key == keyboard::KEY_UP) {
        auto& sel = model->selection;
        
        if (sel.b_line - 1 >= 0) {
            sel.b_index = MIN(int(model->lines[sel.b_line - 1].characters.size()), sel.desired_index);
            sel.b_line -= 1;
        }
        
        if (!(key_state->mods & keyboard::MOD_SHIFT)) {
            sel.a_index = sel.b_index;
            sel.a_line = sel.b_line;
        }
        
        *key_state = { 0 };
    }

    if (key_state->key == keyboard::KEY_DOWN && (key_state->mods & keyboard::MOD_COMMAND)) {
        auto& sel = model->selection;

        if (model->lines.size() == 0) {
            sel.b_index = 0;
            sel.b_line = 0;
        } else {
            sel.b_index = int(model->lines.back().characters.size());
            sel.b_line = int(model->lines.size()) - 1;
        }

        if (!(key_state->mods & keyboard::MOD_SHIFT)) {
            sel.a_index = sel.b_index;
            sel.a_line = sel.b_line;
        }

        *key_state = { 0 };
    }

    if (key_state->key == keyboard::KEY_DOWN) {
        auto& sel = model->selection;
        
        if (sel.b_line + 1 < model->lines.size()) {
            sel.b_index = MIN(int(model->lines[sel.b_line + 1].characters.size()), sel.desired_index);
            sel.b_line += 1;
        }

        if (!(key_state->mods & keyboard::MOD_SHIFT)) {
            sel.a_index = sel.b_index;
            sel.a_line = sel.b_line;
        }
        
        *key_state = { 0 };
    }
    
    if (!readonly && (key_state->key == keyboard::KEY_ENTER || key_state->key == keyboard::KEY_KP_ENTER)) {
        auto& sel = model->selection;
        delete_range(model, sel);
        
        int line  = sel.a_line < sel.b_line ? sel.a_line  : sel.b_line;
        int index = sel.a_line < sel.b_line ? sel.a_index : sel.b_index;
        if (sel.a_line == sel.b_line) {
            index = sel.a_index < sel.b_index ? sel.a_index : sel.b_index;
        }
        
        insert_line_break(model, line, index);
        
        sel.a_line = sel.b_line = line + 1;
        sel.a_index = sel.b_index = 0;
        sel.desired_index = sel.b_index;
        
        *key_state = { 0 };
    }
        
    if (!readonly && (key_state->key == keyboard::KEY_BACKSPACE || key_state->key == keyboard::KEY_DELETE)) {
        auto& sel = model->selection;
        
        if (!range_is_selected) {
            if (key_state->key == keyboard::KEY_BACKSPACE) {
                if (sel.a_index > 0) {
                    sel.a_index -= 1;
                } else if (sel.a_line > 0) {
                    sel.a_line -= 1;
                    sel.a_index = int(model->lines[sel.a_line].characters.size());
                }
            } else {
                if (sel.a_index < model->lines[sel.a_line].characters.size()) {
                    sel.a_index += 1;
                } else if (sel.a_line < model->lines.size() - 1) {
                    sel.a_line += 1;
                    sel.a_index = 0;
                }
            }
        }
        
        delete_range(model, sel);
        
        int line  = sel.a_line < sel.b_line ? sel.a_line  : sel.b_line;
        int index = sel.a_line < sel.b_line ? sel.a_index : sel.b_index;
        if (sel.a_line == sel.b_line) {
            index = sel.a_index < sel.b_index ? sel.a_index : sel.b_index;
        }
        
        sel.a_line  = sel.b_line  = line;
        sel.a_index = sel.b_index = index;
        sel.desired_index = sel.b_index;
        
        *key_state = { 0 };
    }
    
    if (!readonly && key_state->character) {
        auto& sel = model->selection;
        delete_range(model, sel);
        
        int line  = sel.a_line < sel.b_line ? sel.a_line  : sel.b_line;
        int index = sel.a_line < sel.b_line ? sel.a_index : sel.b_index;
        if (sel.a_line == sel.b_line) {
            index = sel.a_index < sel.b_index ? sel.a_index : sel.b_index;
        }
        
        insert_character(model, line, index, key_state->character);
        
        sel.a_line  = sel.b_line  = line;
        sel.a_index = sel.b_index = index + 1;
        sel.desired_index = sel.b_index;
        
        *key_state = { 0 };
    }
    
    if (!readonly && key_state->key == keyboard::KEY_V && (key_state->mods & keyboard::MOD_COMMAND)) {
        auto& sel = model->selection;
        delete_range(model, sel);
        
        int line  = sel.a_line < sel.b_line ? sel.a_line  : sel.b_line;
        int index = sel.a_line < sel.b_line ? sel.a_index : sel.b_index;
        if (sel.a_line == sel.b_line) {
            index = sel.a_index < sel.b_index ? sel.a_index : sel.b_index;
        }
        
        auto pasted_string = get_clipboard_string();
        if (pasted_string) {
            insert_text_content(model, &line, &index, pasted_string);
        }
        
        sel.a_line  = sel.b_line  = line;
        sel.a_index = sel.b_index = index;
        sel.desired_index = index;
        
        *key_state = { 0 };
    }
    
    if (key_state->key == keyboard::KEY_C && (key_state->mods & keyboard::MOD_COMMAND)) {
        if (range_is_selected) {
            auto copied_string = get_text_content(model, model->selection);
            set_clipboard_string(copied_string.get());
            
            *key_state = { 0 };
        }
    }

    if (key_state->key == keyboard::KEY_A && (key_state->mods & keyboard::MOD_COMMAND)) {
        auto& sel = model->selection;

        sel.a_index = 0;
        sel.a_line = 0;
        if (model->lines.size() == 0) {
            sel.b_index = 0;
            sel.b_line = 0;
        } else {
            sel.b_index = int(model->lines.back().characters.size());
            sel.b_line = int(model->lines.size()) - 1;
        }
        sel.desired_index = 0;

        *key_state = { 0 };
    }

    if (!readonly && key_state->key == keyboard::KEY_X && (key_state->mods & keyboard::MOD_COMMAND)) {
        if (range_is_selected) {
            auto& sel = model->selection;
            
            auto copied_string = get_text_content(model, sel);
            set_clipboard_string(copied_string.get());
            
            delete_range(model, sel);
            
            auto min_line  = sel.a_line < sel.b_line ? sel.a_line  : sel.b_line;
            auto min_index = sel.a_line < sel.b_line ? sel.a_index : sel.b_index;
            if (sel.a_line == sel.b_line) {
                min_index = sel.a_index < sel.b_index ? sel.a_index : sel.b_index;
            }
            
            sel.a_line  = sel.b_line  = min_line;
            sel.a_index = sel.b_index = min_index;

            *key_state = { 0 };
        }
    }
    
    model->version_count++;
}

void delete_range(Model* model, const Selection& sel) {
    if (sel.a_line == sel.b_line) {
        if (sel.a_index == sel.b_index) {
            return; // Nothing to delete
        }
    
        // Single-line range to remove
        auto& line = model->lines[sel.a_line];
        auto  from = sel.a_index < sel.b_index ? sel.a_index : sel.b_index;
        auto  to   = sel.a_index > sel.b_index ? sel.a_index : sel.b_index;

        auto ch_from = from == 0 ? 0 : line.characters[from - 1].index + line.characters[from - 1].num_bytes;
        auto ch_to   = to   == 0 ? 0 : line.characters[to   - 1].index + line.characters[to   - 1].num_bytes;
        auto ch_num  = ch_to - ch_from;

        // Update the content array
        auto new_num_bytes = line.num_bytes - (ch_to - ch_from);
        auto new_content = new char[new_num_bytes];
        strncpy(new_content, line.content.get(), ch_from);
        strncpy(new_content + ch_from, line.content.get() + ch_to, line.num_bytes - ch_to);
        line.content = std::unique_ptr<char[]>(new_content);
        line.num_bytes = new_num_bytes;
        
        // Update the character indices
        line.characters.erase(line.characters.begin() + from, line.characters.begin() + to);
        for (auto it = line.characters.begin() + from; it < line.characters.end(); ++it) {
            it->index -= ch_num;
        }
        
        // Update start-of-line styles
        if (from == 0 && !line.characters.empty()) {
            line.style = line.characters.front().style;
        }
    
    } else {
        // Multi-line range to remove
        auto  from_lineno = sel.a_line < sel.b_line ? sel.a_line  : sel.b_line;
        auto  from_index  = sel.a_line < sel.b_line ? sel.a_index : sel.b_index;
        auto& from_line   = model->lines[from_lineno];
        
        auto  to_lineno = sel.a_line > sel.b_line ? sel.a_line  : sel.b_line;
        auto  to_index  = sel.a_line > sel.b_line ? sel.a_index : sel.b_index;
        auto& to_line   = model->lines[to_lineno];
        
        auto ch_from = from_index == 0 ? 0 : from_line.characters[from_index - 1].index +
                                             from_line.characters[from_index - 1].num_bytes;
        
        
        auto ch_to = to_index == 0 ? 0 : to_line.characters[to_index - 1].index +
                                         to_line.characters[to_index - 1].num_bytes;
        
        // Update the content array
        auto new_num_bytes = ch_from + (to_line.num_bytes - ch_to);
        auto new_content = new char[new_num_bytes];
        strncpy(new_content, from_line.content.get(), ch_from);
        strncpy(new_content + ch_from, to_line.content.get() + ch_to, to_line.num_bytes - ch_to);
        from_line.content = std::unique_ptr<char[]>(new_content);
        from_line.num_bytes = new_num_bytes;
        
        // Update the character indices
        from_line.characters.erase(from_line.characters.begin() + from_index, from_line.characters.end());
        for (auto it = to_line.characters.begin() + to_index; it < to_line.characters.end(); ++it) {
            it->index = it->index - ch_to + ch_from;
            from_line.characters.push_back(*it);
        }
        
        // Remove lines
        model->lines.erase(model->lines.begin() + from_lineno + 1, model->lines.begin() + to_lineno + 1);
        
        // Update start-of-line styles
        if (from_index == 0 && !from_line.characters.empty()) {
            from_line.style = from_line.characters.front().style;
        }
    }
    
    model->version_count++;
}

void insert_character(Model* model, int lineno, int index, const char* character) {
    auto length = (
        !(*character & 0x80) ? 1 :
        !(*character & 0x20) ? 2 :
        !(*character & 0x10) ? 3 : 4
    );

    auto& line = model->lines[lineno];

    auto ch_index = index == 0 ? 0 : line.characters[index - 1].index + line.characters[index - 1].num_bytes;

    // Update the content array
    auto new_num_bytes = line.num_bytes + length;
    auto new_content = new char[new_num_bytes];
    strncpy(new_content, line.content.get(), ch_index);
    strncpy(new_content + ch_index, character, length);
    strncpy(new_content + ch_index + length, line.content.get() + ch_index, line.num_bytes - ch_index);
    line.content = std::unique_ptr<char[]>(new_content);
    line.num_bytes = new_num_bytes;

    // Insert the new character
    Character character_entry;
    character_entry.index = ch_index;
    character_entry.num_bytes = length;
    character_entry.entity_id = -1;
    if (index == 0) {
        character_entry.style = line.style;
    } else {
        auto& prev_character = line.characters[index - 1];
        character_entry.style = prev_character.style;
    }
    line.characters.insert(line.characters.begin() + index, character_entry);

    // Update the character indices
    for (auto it = line.characters.begin() + index + 1; it < line.characters.end(); ++it) {
        it->index += length;
    }

    model->version_count++;
}

void insert_line_break(Model* model, int lineno, int index) {
    auto& line = model->lines[lineno];
    
    auto ch_index = index == 0 ? 0 : line.characters[index - 1].index + line.characters[index - 1].num_bytes;
    
    // Create two new content arrays
    auto a_num_bytes = ch_index + 1;
    auto a_content = new char[a_num_bytes];
    strncpy(a_content, line.content.get(), ch_index);
    a_content[ch_index] = '\0';
    
    auto b_num_bytes = line.num_bytes - ch_index;
    auto b_content = new char[b_num_bytes];
    strncpy(b_content, line.content.get() + ch_index, line.num_bytes - ch_index);
    
    line.content = std::unique_ptr<char[]>(a_content);
    line.num_bytes = a_num_bytes;
    
    Line new_line;
    new_line.content = std::unique_ptr<char[]>(b_content);
    new_line.num_bytes = b_num_bytes;
    
    // Move the characters over to the new line
    new_line.characters = std::vector<Character>(line.characters.begin() + index, line.characters.end());
    line.characters.erase(line.characters.begin() + index, line.characters.end());
    for (auto& character : new_line.characters) {
        character.index -= ch_index;
    }
    
    // Update start-of-line styles
    if (!new_line.characters.empty()) {
        new_line.style = new_line.characters.front().style;
    } else if (!line.characters.empty()) {
        new_line.style = line.characters.back().style;
    } else {
        new_line.style = line.style;
    }
    
    // Insert the new line
    model->lines.insert(model->lines.begin() + lineno + 1, std::move(new_line));
    model->version_count++;
}

void remove_line_breaks(Model* model) {
    if (model->lines.size() <= 1) {
        return;
    }

    // Step 1. Make the selection single-line
    int a_index = 0;
    for (int lineno = 0; lineno < model->selection.a_line; ++lineno) {
        a_index += model->lines[lineno].characters.size();
    }
    a_index += model->selection.a_index;

    int b_index = 0;
    for (int lineno = 0; lineno < model->selection.b_line; ++lineno) {
        b_index += model->lines[lineno].characters.size();
    }
    b_index += model->selection.b_index;

    model->selection.a_line = model->selection.b_line = 0;
    model->selection.a_index = a_index;
    model->selection.b_index = b_index;

    // Step 2. Create the new line
    Line single_line;
    {
        int num_bytes = 0;
        for (auto& line : model->lines) {
            num_bytes += line.num_bytes - 1;
        }
        num_bytes += 1;
        
        auto content = new char[num_bytes];
        int offset = 0;
        
        for (auto& line : model->lines) {
            strncpy(content + offset, line.content.get(), line.num_bytes - 1);
            for (auto character : line.characters) {
                character.index += offset;
                single_line.characters.push_back(character);
            }
            offset += line.num_bytes - 1;
        }
        
        content[num_bytes - 1] = '\0';
        
        single_line.num_bytes = num_bytes;
        single_line.content = std::unique_ptr<char[]>(content);
    }
    
    // Done.
    model->lines.clear();
    model->lines.push_back(std::move(single_line));
    model->version_count++;
}

}
