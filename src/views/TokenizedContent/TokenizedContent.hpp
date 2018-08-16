//
//  TokenizedContent.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 15/08/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef TokenizedContent_hpp
#define TokenizedContent_hpp

#include <ddui/Context>
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

    // type = TEXT
    struct {
        NVGcolor font_color;
        float font_size;
        const char* font_face;
        std::string content;
    } text;

    // type = OBJECT
    struct {
        float width;
        float height;
        std::function<void(Context ctx)> update;
    } object;

};

struct State {
    std::vector<Token> content_tokens;

    struct {
        NVGcolor font_color;
        float font_size;
        const char* font_face;
    } settings;
};

void set_font_settings(State* state, NVGcolor font_color, float font_size, const char* font_face);
void tokenize_and_append_text(State* state, std::string text);
void tokenize_and_append_text(State* state, float space_before, float space_after, std::string text);
void append_object(State* state, float width, float height, std::function<void(Context ctx)> update);
void append_object(State* state, float space_before, float space_after, float width, float height, std::function<void(Context ctx)> update);

float measure_content_height(State* state, Context ctx, float total_width);
void update(State* state, Context ctx);

}

#endif
