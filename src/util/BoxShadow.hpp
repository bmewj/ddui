//
//  BoxShadow.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 26/07/2019.
//  Copyright © 2019 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_BoxShadow_hpp
#define ddui_BoxShadow_hpp

#include <ddui/core>

namespace ddui {

struct BoxShadow {
    BoxShadow& offset(float offset_x, float offset_y) {
        offset_x_ = offset_x;
        offset_y_ = offset_y;
        return *this;
    }
    BoxShadow& blur(float blur) {
        blur_ = blur;
        return *this;
    }
    BoxShadow& border_radius(float border_radius) {
        border_radius_ = border_radius;
        return *this;
    }
    BoxShadow& color(Color color) {
        color_ = color;
        return *this;
    }
    BoxShadow& opacity(float opacity) {
        opacity_ = opacity;
        return *this;
    }
    void render() {

        auto color_interior = color_;
        auto color_exterior = color_;
        color_interior.a = opacity_;
        color_exterior.a = 0.0;

        auto paint = ddui::box_gradient(
            offset_x_, offset_y_,
            view.width, view.height,
            border_radius_,
            2 * blur_,
            color_interior,
            color_exterior
        );

        begin_path();
        rect(- blur_ + offset_x_, - blur_ + offset_y_, view.width + 2 * blur_, view.height + 2 * blur_);
        fill_paint(paint);
        fill();
    }

private:
    float offset_x_      = 0.0;
    float offset_y_      = 0.0;
    float blur_          = 20.0;
    float border_radius_ = 0.0;
    Color color_         = rgb(0x000000);
    float opacity_       = 0.2;
};

}

#endif
