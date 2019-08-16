//
//  TokenizedTextBox.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 06/08/2019.
//  Copyright © 2019 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_TokenizedTextBox_hpp
#define ddui_TokenizedTextBox_hpp

#include <ddui/views/PlainTextBox>
#include <string>

struct IAutoCompletionController {
    virtual void set_anchor_point(float x, float y) = 0;
    virtual void set_user_text   (const std::string& text) = 0;
    virtual void on_user_key_up  () = 0;
    virtual void on_user_key_down() = 0;
    virtual bool        has_auto_completion_string() = 0;
    virtual std::string get_auto_completion_string() = 0;
};

struct TokenizedTextBox : PlainTextBox {

    // Styles
    struct TokenStyleOptions {
        ddui::Color background_color;
        ddui::Color text_color;
        const char* font_face;
        float       font_size;
        float       border_radius;
        float       padding;
        float       margin;
    };
    static TokenStyleOptions* get_global_token_styles();

    // Methods
    TokenizedTextBox(State* state, Model* model);
    TokenizedTextBox& set_styles(const StyleOptions* styles);
    TokenizedTextBox& set_token_styles(const TokenStyleOptions* styles);
    TokenizedTextBox& set_multiline(bool multiline);
    TokenizedTextBox& token_separator(const char* separator);
    TokenizedTextBox& auto_completion_controller(IAutoCompletionController& ac_controller);
    void update();

    // Helpers
    static void extract_tokens(const Model* model, const char* separator, std::vector<std::string>* out);

protected:
    const char* separator = ";";
    IAutoCompletionController* ac_controller;
    const TokenStyleOptions* token_styles;

    void process_key_input() override;
    void refresh_model_measurements() override;
    void measure_entity(int line, int index, int entity_id, float* width, float* height) override;
    void draw_entity   (int line, int index, int entity_id) override;
    void get_user_input_selection(TextEdit::Selection* selection);
    void tokenize_content();
};

#endif
