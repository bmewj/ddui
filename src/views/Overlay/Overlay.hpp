//
//  Overlay.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 10/07/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_Overlay_hpp
#define ddui_Overlay_hpp

#include <ddui/core>
#include <functional>

namespace Overlay {

enum InputBlockSetting {
    BLOCKS_INPUT,
    DOESNT_BLOCK_INPUT
};

void update(std::function<void()> inner_update);
void handle_overlay(void* identifier, std::function<void()> inner_update);
void open(void* identifier, InputBlockSetting input_block_setting = BLOCKS_INPUT);
void close(void* identifier);
bool is_open(void* identifier);

}

#endif
