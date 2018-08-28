//
//  TokenizedContent.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 15/08/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef TokenizedContent_hpp
#define TokenizedContent_hpp

#include <ddui/core>
#include <functional>
#include <vector>
#include <string>

namespace TokenizedContent {

struct Token {

    enum Type {
        TEXT,
        OBJECT,
        LINE_BREAK
    };

    Type type;
    float space_before, space_after;
    int action_id;

    // type = TEXT
    struct {
        ddui::Color font_color;
        float font_size;
        const char* font_face;
        std::string content;
    } text;

    // type = OBJECT
    struct {
        float width;
        float height;
        std::function<void()> update;
    } object;

};

struct State {
    std::vector<Token> content_tokens;
    int action;

    struct {
        ddui::Color font_color;
        float font_size;
        const char* font_face;
    } settings;
};

void set_font_settings(State* state, ddui::Color font_color, float font_size, const char* font_face);
void tokenize_and_append_text(State* state, std::string text, int action_id = -1);
void tokenize_and_append_text(State* state, float space_before, float space_after, std::string text, int action_id = -1);
void append_object(State* state, float width, float height, std::function<void()> update, int action_id = -1);
void append_object(State* state, float space_before, float space_after, float width, float height, std::function<void()> update, int action_id = -1);

float measure_content_height(State* state, float total_width);
void update(State* state);

}

#endif
