//
//  TokenizedContent.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 15/08/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "TokenizedContent.hpp"

namespace TokenizedContent {

using namespace ddui;

constexpr float SPACE_WIDTH = 3.0;

void set_font_settings(State* state, Color font_color, float font_size, const char* font_face) {
    state->settings.font_color = font_color;
    state->settings.font_size = font_size;
    state->settings.font_face = font_face;
}

void tokenize_and_append_text(State* state, std::string text, int action_id) {
    tokenize_and_append_text(state, SPACE_WIDTH, SPACE_WIDTH, std::move(text), action_id);
}

void tokenize_and_append_text(State* state, float space_before, float space_after, std::string text, int action_id) {

    int start_index = 0;
    int index = 0;

    std::vector<std::string> parts;

    // Separate by spaces
    while (index < text.size()) {
        auto ch = text[index];
        if (ch == ' ') {
            parts.push_back(text.substr(start_index, index - start_index));
            start_index = index + 1;
        }
        if (ch == '\n') {
            parts.push_back(text.substr(start_index, index - start_index));
            parts.push_back("\n");
            start_index = index + 1;
        }
        ++index;
    }
    if (start_index < index) {
        parts.push_back(text.substr(start_index, index - start_index));
    }

    for (auto& part : parts) {
        
        if (part == "\n") {
            Token token;
            token.type = Token::LINE_BREAK;
            state->content_tokens.push_back(std::move(token));
            continue;
        }

        if (part == "") {
            continue;
        }

        Token token;
        token.type = Token::TEXT;
        token.space_before = (&part == &parts.front()) ? space_before : SPACE_WIDTH;
        token.space_after  = (&part == &parts.back())  ? space_after  : SPACE_WIDTH;
        token.action_id = action_id;
        token.text.font_color = state->settings.font_color;
        token.text.font_size = state->settings.font_size;
        token.text.font_face = state->settings.font_face;
        token.text.content = std::move(part);
        state->content_tokens.push_back(std::move(token));
    }

}

void append_object(State* state, float width, float height, std::function<void()> update, int action_id) {
    append_object(state, SPACE_WIDTH, SPACE_WIDTH, width, height, std::move(update), action_id);
}

void append_object(State* state, float space_before, float space_after, float width, float height, std::function<void()> update, int action_id) {
    Token token;
    token.type = Token::OBJECT;
    token.space_before = space_before;
    token.space_after  = space_after;
    token.action_id = action_id;
    token.object.width = width;
    token.object.height = height;
    token.object.update = std::move(update);
    state->content_tokens.push_back(std::move(token));
}

static float measure_content(State* state, float total_width, float* xs, float* ys, float* ws, float* hs) {
    float y = 0;

    int i = 0;
    while (i < state->content_tokens.size()) {

        int line_start_index = i;
        float x = 0;
        float max_height = 0;
        float max_line_height = 0;
        float max_ascender = 0;

        // Fit tokens on one line
        for (; i < state->content_tokens.size(); ++i) {
            auto& token = state->content_tokens[i];

            bool end_of_line = false;
            switch (token.type) {

                case Token::TEXT: {

                    // Measure text
                    font_face(token.text.font_face);
                    font_size(token.text.font_size);

                    float ascender, descender, line_height;
                    text_metrics(&ascender, &descender, &line_height);

                    float bounds[4];
                    text_bounds(0, 0, token.text.content.c_str(), NULL, bounds);

                    auto width = bounds[2] - bounds[0];

                    if (x + width > total_width && x > 0) {
                        end_of_line = true;
                        break;
                    }

                    xs[i] = x;
                    ws[i] = width;

                    if (max_line_height < line_height) {
                        max_line_height = line_height;
                    }
                    if (max_ascender < ascender) {
                        max_ascender = ascender;
                    }
                    if (max_height < line_height) {
                        max_height = line_height;
                    }

                    auto space = token.space_after;
                    if (i + 1 < state->content_tokens.size() &&
                        space < state->content_tokens[i + 1].space_before) {
                        space = state->content_tokens[i + 1].space_before;
                    }

                    x += width + space;

                    break;
                }

                case Token::OBJECT: {

                    if (x + token.object.width > total_width && x > 0) {
                        end_of_line = true;
                        break;
                    }

                    xs[i] = x;
                    ws[i] = token.object.width;

                    if (max_height < token.object.height) {
                        max_height = token.object.height;
                    }

                    auto space = token.space_after;
                    if (i + 1 < state->content_tokens.size() &&
                        space < state->content_tokens[i + 1].space_before) {
                        space = state->content_tokens[i + 1].space_before;
                    }

                    x += token.object.width + space;

                    break;
                }

                case Token::LINE_BREAK: {
                    end_of_line = true;
                    ++i;
                    break;
                }

            }

            if (end_of_line) {
                break;
            }
        }

        // Calculate baseline
        float baseline = (max_height - max_line_height) / 2 + max_ascender;

        // Update baselines for all tokens
        for (int j = line_start_index; j < i; ++j) {
            auto& token = state->content_tokens[j];
            
            switch (token.type) {
                case Token::TEXT: {
                    ys[j] = y + baseline;
                    hs[j] = max_ascender;
                    break;
                }
                case Token::OBJECT: {
                    auto height = token.object.height;
                    if (height < max_ascender) {
                        ys[j] = y + baseline - height;
                    } else {
                        ys[j] = y + (max_height - height) / 2;
                    }
                    hs[j] = max_height;
                    break;
                }
                case Token::LINE_BREAK: {
                    break;
                }
            }
        }
        y += max_height;

    }

    return y;
}

float measure_content_height(State* state, float total_width) {
    float xs[state->content_tokens.size()];
    float ys[state->content_tokens.size()];
    float ws[state->content_tokens.size()];
    float hs[state->content_tokens.size()];
    return measure_content(state, total_width, xs, ys, ws, hs);
}

void update(State* state) {
    float xs[state->content_tokens.size()];
    float ys[state->content_tokens.size()];
    float ws[state->content_tokens.size()];
    float hs[state->content_tokens.size()];
    measure_content(state, view.width, xs, ys, ws, hs);

    text_align(align::BASELINE | align::LEFT);
    state->action = -1;

    for (int i = 0; i < state->content_tokens.size(); ++i) {
        auto& token = state->content_tokens[i];

        switch (token.type) {
        
            case Token::TEXT: {
                font_face(token.text.font_face);
                font_size(token.text.font_size);
                fill_color(token.text.font_color);
                text(xs[i], ys[i], token.text.content.c_str(), NULL);
                if (token.action_id != -1) {
                    if (mouse_over(xs[i], ys[i] - hs[i], ws[i], hs[i])) {
                        set_cursor(CURSOR_POINTING_HAND);
                    }
                    if (mouse_hit(xs[i], ys[i] - hs[i], ws[i], hs[i])) {
                        mouse_hit_accept();
                        state->action = token.action_id;
                    }
                }
                break;
            }
            
            case Token::OBJECT: {
                sub_view(xs[i], ys[i], token.object.width, token.object.height);
                token.object.update();
                restore();
                if (token.action_id != -1) {
                    if (mouse_over(xs[i], ys[i], ws[i], hs[i])) {
                        set_cursor(CURSOR_POINTING_HAND);
                    }
                    if (mouse_hit(xs[i], ys[i], ws[i], hs[i])) {
                        mouse_hit_accept();
                        state->action = token.action_id;
                    }
                }
                break;
            }
            
            case Token::LINE_BREAK: {
                break;
            }
            
        }
        
        if (token.action_id != -1) {
            
        }
    }
}

}
