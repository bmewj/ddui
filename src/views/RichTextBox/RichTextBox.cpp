//
//  RichTextBox.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 19/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "RichTextBox.hpp"

using namespace ddui;

static void apply_rich_text_commands(TextEdit::Model* model, KeyState* key);

void RichTextBox::process_key_input() {
    if (multiline ||
        (key_state.key != keyboard::KEY_ENTER &&
         key_state.key != keyboard::KEY_KP_ENTER)) {
        TextEdit::apply_keyboard_input(&model, &key_state);
    }
    apply_rich_text_commands(&model, &key_state);
    if (!multiline) {
        TextEdit::remove_line_breaks(&model);
    }
}

void apply_rich_text_commands(TextEdit::Model* model, KeyState* key) {

    if (key->action != keyboard::ACTION_PRESS) {
        return;
    }

    auto& sel = model->selection;

    auto min_line  = sel.a_line < sel.b_line ? sel.a_line  : sel.b_line;
    auto min_index = sel.a_line < sel.b_line ? sel.a_index : sel.b_index;
    if (sel.a_line == sel.b_line) {
        min_index = sel.a_index < sel.b_index ? sel.a_index : sel.b_index;
    }

    if ((key->mods & keyboard::MOD_COMMAND) &&
        (key->key == keyboard::KEY_B)) {
        
        auto& line = model->lines[min_line];
        if (min_index < line.characters.size()) {
            TextEdit::StyleCommand style;
            style.type = TextEdit::StyleCommand::BOLD;
            style.bool_value = !line.characters[min_index].style.font_bold;
            TextEdit::apply_style(model, sel, style);
        }
    }

    if ((key->mods & keyboard::MOD_COMMAND) &&
        (key->mods & keyboard::MODIFIER_SHIFT) &&
        (key->key == keyboard::KEY_EQUAL)) {
        
        auto& line = model->lines[min_line];
        if (min_index < line.characters.size()) {
            TextEdit::StyleCommand style;
            style.type = TextEdit::StyleCommand::SIZE;
            style.float_value = line.characters[min_index].style.text_size * 1.1;
            TextEdit::apply_style(model, sel, style);
        }
    }

    if ((key->mods & keyboard::MOD_COMMAND) &&
        (key->key == keyboard::KEY_MINUS)) {
        
        auto& line = model->lines[min_line];
        if (min_index < line.characters.size()) {
            TextEdit::StyleCommand style;
            style.type = TextEdit::StyleCommand::SIZE;
            style.float_value = line.characters[min_index].style.text_size * (1 / 1.1);
            TextEdit::apply_style(model, sel, style);
        }
    }
}
