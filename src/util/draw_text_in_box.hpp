//
//  draw_text_in_box.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 11/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_draw_text_in_box_hpp
#define ddui_draw_text_in_box_hpp

#include <nanovg.h>

void draw_text_in_box(NVGcontext* vg, int x, int y, int width, int height, const char* content);
int truncate_text(NVGcontext* vg, int width, int strlen, char* dst, const char* src);

#endif
