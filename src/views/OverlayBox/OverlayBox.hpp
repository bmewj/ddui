//
//  OverlayBox.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 06/08/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef OverlayBox_hpp
#define OverlayBox_hpp

#include <functional>
#include <ddui/core>

namespace OverlayBox {

struct OverlayBoxState {
    OverlayBoxState();

    float max_width;
    float max_height;
    std::function<void()> update_content;

};

void open(OverlayBoxState* state);
void close(OverlayBoxState* state);
void update(OverlayBoxState* state, std::function<void()> update_content);

}

#endif
