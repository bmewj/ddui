//
//  MessageBox.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 06/08/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef MessageBox_hpp
#define MessageBox_hpp

#include <ddui/views/OverlayBox>
#include <string>

namespace MessageBox {

struct MessageBoxState {
    MessageBoxState();

    OverlayBox::OverlayBoxState overlay_box;
    std::string title;
    std::string message;
    std::vector<std::string> button_set;
    bool can_dismiss;
};

void open(MessageBoxState* state);
void close(MessageBoxState* state);
void update(MessageBoxState* state);

}

#endif
