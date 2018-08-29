//
//  draw_text_in_box.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 11/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_draw_text_in_box_hpp
#define ddui_draw_text_in_box_hpp

void draw_text_in_box(float x, float y, float width, float height, const char* content);
void draw_centered_text_in_box(float x, float y, float width, float height, const char* content);
int truncate_text(float width, int strlen, char* dst, const char* src);

#endif
