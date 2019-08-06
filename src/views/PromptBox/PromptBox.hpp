//
//  PromptBox.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 06/08/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef PromptBox_hpp
#define PromptBox_hpp

#include <ddui/views/OverlayBox>
#include <ddui/views/PlainTextBox>
#include <string>

namespace PromptBox {

struct PromptBoxState {
    PromptBoxState();

    OverlayBox::OverlayBoxState overlay_box;
    std::string title;
    std::string message;
    TextEdit::Model text_box_model;
    PlainTextBox::State text_box_state;
    std::vector<std::string> button_set;
    bool can_dismiss;
    bool opened;
    
    int action;
};

void open(PromptBoxState* state);
void close(PromptBoxState* state);
void update(PromptBoxState* state);

}

#endif
