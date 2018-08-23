//
//  Overlay.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 10/07/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_Overlay_hpp
#define ddui_Overlay_hpp

#include <ddui/ddui>
#include <functional>

namespace Overlay {

void update(std::function<void()> inner_update);
void handle_overlay(void* identifier, std::function<void()> inner_update);
void open(void* identifier);
void close(void* identifier);
bool is_open(void* identifier);

}

#endif
