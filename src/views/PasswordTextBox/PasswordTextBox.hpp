//
//  PasswordTextBox.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 20/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_PasswordTextBox_hpp
#define ddui_PasswordTextBox_hpp

#include <ddui/views/PlainTextBox>

struct PasswordTextBox : PlainTextBox {
    PasswordTextBox(State* state, Model* model) : PlainTextBox(state, model) {}

protected:
    void refresh_model_measurements() override;
    void draw_content() override;
};

#endif
