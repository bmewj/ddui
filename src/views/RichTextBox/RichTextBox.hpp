//
//  RichTextBox.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 19/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_RichTextBox_hpp
#define ddui_RichTextBox_hpp

#include <ddui/views/PlainTextBox>

struct RichTextBox : PlainTextBox {
    RichTextBox(State* state, Model* model) : PlainTextBox(state, model) {}

protected:
    void process_key_input() override;
};

#endif
