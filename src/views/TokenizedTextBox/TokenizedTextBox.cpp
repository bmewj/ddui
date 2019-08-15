//
//  TokenizedTextBox.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 06/08/2019.
//  Copyright © 2019 Bartholomew Joyce All rights reserved.
//

#include "TokenizedTextBox.hpp"
#include <cstring>

struct DummyAutoCompletionController : IAutoCompletionController {
    void set_anchor_point(float x, float y) override {}
    void set_user_text(const std::string& text) override {}
    void on_user_key_up() override {}
    void on_user_key_down() override {}
    bool has_auto_completion_string() override {
        return false;
    }
    std::string get_auto_completion_string() override {
        return "";
    }
};

TokenizedTextBox::TokenStyleOptions* TokenizedTextBox::get_global_token_styles() {
    static TokenStyleOptions styles;
    static bool did_init = false;
    if (did_init) {
        return &styles;
    }

    did_init = true;
    styles.background_color = ddui::rgb(0xdddddd);
    styles.text_color       = ddui::rgb(0x444444);
    styles.font_face        = "regular";
    styles.font_size        = 16.0;
    styles.border_radius    = 2.0;
    styles.padding          = 1.0;
    styles.margin           = 1.0;
    return &styles;
}

TokenizedTextBox::TokenizedTextBox(State* state, Model* model) : PlainTextBox(state, model) {
    static DummyAutoCompletionController dummy_ac_controller;
    ac_controller = &dummy_ac_controller;

    token_styles = get_global_token_styles();
}

TokenizedTextBox& TokenizedTextBox::set_styles(const StyleOptions* styles) {
    PlainTextBox::set_styles(styles);
    return *this;
}

TokenizedTextBox& TokenizedTextBox::set_token_styles(const TokenStyleOptions* token_styles) {
    this->token_styles = token_styles;
    return *this;
}

TokenizedTextBox& TokenizedTextBox::set_multiline(bool multiline) {
    PlainTextBox::set_multiline(multiline);
    return *this;
}

TokenizedTextBox& TokenizedTextBox::token_separator(const char* separator) {
    this->separator = separator;
    return *this;
}

TokenizedTextBox& TokenizedTextBox::auto_completion_controller(IAutoCompletionController& ac_controller) {
    this->ac_controller = &ac_controller;
    return *this;
}

void TokenizedTextBox::update() {
    // We want to detect external modification of the model. Usually, the model
    // will only be changed from within our update function by us processing user
    // input. In this case, the previous_version_count will always == the model
    // version count. They will only go out of sync during the update process
    // (after we process key input the model count increases, after we re-measure
    // the content our version_count increases).
    //
    // In the event that our version count is not matching the model version count
    // at the start of an update, then the model has been tampered with by some
    // external piece of code. This should trigger a complete re-parsing of the
    // text box, parsing the separators into tokens.
    if (state.current_version_count != model.version_count) {
        tokenize_content();
    } 

    PlainTextBox::update();
}

void TokenizedTextBox::process_key_input() {

    // Get the selection/range of text that the user is currently editing
    TextEdit::Selection user_input_selection;
    get_user_input_selection(&user_input_selection);

    // Extract the content from in the range
    std::string user_input;
    {
        auto str = TextEdit::get_text_content(&model, user_input_selection);
        user_input = str.get();
    }

    // Pressing ENTER should be considered equivalent to inserting a separator
    if ((!user_input.empty() || ac_controller->has_auto_completion_string()) &&
        ddui::key_state.action == ddui::keyboard::ACTION_PRESS &&
        (ddui::key_state.key   == ddui::keyboard::KEY_ENTER ||
         ddui::key_state.key   == ddui::keyboard::KEY_KP_ENTER)) {
        ddui::consume_key_event();
        auto& sel = user_input_selection;

        // If the user has selected an item in the auto complete list we want
        // to substitute it in right now.
        if (ac_controller->has_auto_completion_string()) {
            const auto& substitute = ac_controller->get_auto_completion_string();
            TextEdit::delete_range(&model, sel);
            sel.b_index = sel.a_index;
            TextEdit::insert_text_content(&model, &sel.b_line, &sel.b_index, substitute.c_str());
        }

        // We want to insert a separator character directly after the user input
        TextEdit::insert_character(&model, sel.b_line, sel.b_index, separator);
        sel.b_index++;

        // Now that we've added our separator character, we want to convert the
        // entire selection into an entity character
        TextEdit::create_entity(&model, sel.a_line, sel.a_index, sel.b_index, 1);

        return;
    }

    // Pressing the UP & DOWN arrows should be used to navigate the auto complete list
    if (ddui::key_state.action != ddui::keyboard::ACTION_RELEASE &&
        (ddui::key_state.key   == ddui::keyboard::KEY_UP ||
         ddui::key_state.key   == ddui::keyboard::KEY_DOWN)) {
        if (ddui::key_state.key == ddui::keyboard::KEY_UP) {
            ac_controller->on_user_key_up();
        } else {
            ac_controller->on_user_key_down();
        }
        ddui::consume_key_event();

        return;
    }

    // If the user is pasting content, we need to tokenize the model once they've pasted
    // their new text.
    bool did_paste = (
        ddui::key_state.action != ddui::keyboard::ACTION_RELEASE &&
        ddui::key_state.key    == ddui::keyboard::KEY_V &&
        (ddui::key_state.mods   & ddui::keyboard::MOD_COMMAND)
    );
    
    // If the user is typing the separator character, we need to tokenize the model as well
    bool did_type_separator = (
        ddui::key_state.action != ddui::keyboard::ACTION_RELEASE &&
        ddui::key_state.character &&
        std::strcmp(ddui::key_state.character, separator) == 0
    );

    PlainTextBox::process_key_input();

    if (did_paste || did_type_separator) {
        tokenize_content();
    }
}

void TokenizedTextBox::refresh_model_measurements() {
    // When the text has changed, we want to refresh the auto complete list
    // to show new options.

    PlainTextBox::refresh_model_measurements();

    // Get the selection/range of text that the user is currently editing
    TextEdit::Selection user_input_selection;
    get_user_input_selection(&user_input_selection);

    // Extract the content from in the range
    std::string user_input;
    {
        auto str = TextEdit::get_text_content(&model, user_input_selection);
        user_input = str.get();
    }

    // Calculate the x, y anchor point
    float x, y;
    {
        auto& sel = user_input_selection;

        const auto& line = state.measurements.lines[sel.a_line];
        y = line.y + line.height + styles->margin;

        if (sel.a_index > 0) {
            x = line.characters[sel.a_index - 1].max_x;
        } else {
            x = 0;
        }

        // Adjust x position to account for margin and scroll
        x += styles->margin - state.scroll_x;
        if (x < 0) {
            x = 0;
        }
    }
    ac_controller->set_anchor_point(x, y);

    // Re-build the auto complete list
    ac_controller->set_user_text(user_input);
}

void TokenizedTextBox::measure_entity(int line, int index, int entity_id, float* width, float* height) {
    ddui::font_face(token_styles->font_face);
    ddui::font_size(token_styles->font_size);

    float ascender, descender, line_height;
    ddui::text_metrics(&ascender, &descender, &line_height);

    const auto& entity_ch = model.lines[line].characters[index];
    const char* entity_ptr = model.lines[line].content.get() + entity_ch.index;
    const char* entity_ptr_end = entity_ptr + entity_ch.num_bytes - std::strlen(separator) - 1;

    float bounds[4];
    ddui::text_bounds(0, 0, entity_ptr, entity_ptr_end, bounds);

    *width = bounds[2] - bounds[1] + token_styles->margin * 2 + token_styles->padding * 2;
    *height = line_height;
}

void TokenizedTextBox::draw_entity(int line, int index, int entity_id) {
    // Background
    ddui::begin_path();
    ddui::fill_color(token_styles->background_color);
    ddui::rounded_rect(
        token_styles->margin,
        -token_styles->padding,
        ddui::view.width - 2 * token_styles->margin,
        ddui::view.height + 2 * token_styles->padding,
        token_styles->border_radius
    );
    ddui::fill();

    // Text content
    ddui::font_face(token_styles->font_face);
    ddui::font_size(token_styles->font_size);
    ddui::fill_color(token_styles->text_color);
    ddui::text_align(ddui::align::CENTER | ddui::align::MIDDLE);
    
    const auto& entity_ch = model.lines[line].characters[index];
    const char* entity_ptr = model.lines[line].content.get() + entity_ch.index;
    const char* entity_ptr_end = entity_ptr + entity_ch.num_bytes - std::strlen(separator);
    ddui::text(0.5 * ddui::view.width, 0.5 * ddui::view.height, entity_ptr, entity_ptr_end);
}

void TokenizedTextBox::get_user_input_selection(TextEdit::Selection* selection) {
    const auto lineno = model.selection.a_line;
    const auto& chrs = model.lines[lineno].characters;

    auto a_index = model.selection.a_index;
    auto b_index = model.selection.a_index;

    for (; a_index > 0           && chrs[a_index - 1].entity_id == -1; --a_index) {}
    for (; b_index < chrs.size() && chrs[b_index    ].entity_id == -1; ++b_index) {}

    selection->a_line  = lineno;
    selection->b_line  = lineno;
    selection->a_index = a_index;
    selection->b_index = b_index;
}

void TokenizedTextBox::tokenize_content() {
    const int separator_len = std::strlen(separator);

    for (int lineno = 0; lineno < model.lines.size(); ++lineno) {
        auto& line = model.lines[lineno];
        int index = 0;
        while (true) {
            const char* content = line.content.get();
            const auto& chrs = line.characters;
            const int numchars = chrs.size();

            // Find first non-entity character
            for (; index < numchars && chrs[index].entity_id != -1; ++index) {}
            if (index == numchars) {
                break;
            }

            int start_index = index;

            // Find a separator character
            for (; index < numchars && chrs[index].entity_id == -1; ++index) {
                if (chrs[index].num_bytes == separator_len &&
                    std::strncmp(separator, content + chrs[index].index, separator_len) == 0) {
                    break;
                }
            }
            if (index == numchars || chrs[index].entity_id != -1) {
                continue;
            }
            
            // We have found a separator at index, create an entity
            TextEdit::create_entity(&model, lineno, start_index, index + 1, 1);
            index = start_index + 1;
        }
    }
}
