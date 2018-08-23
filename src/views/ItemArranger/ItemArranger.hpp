//
//  ItemArranger.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 21/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_ItemArranger_hpp
#define ddui_ItemArranger_hpp

#include <ddui/ddui>
#include <string>
#include <vector>

namespace ItemArranger {

class Model {
    public:
        virtual int count() = 0;
        virtual std::string label(int index) = 0;
        virtual bool get_enabled(int index) = 0;
        virtual void set_enabled(int index, bool enabled) = 0;
        virtual void reorder(int old_index, int new_index) = 0;
};

class BasicModel : public Model {
    public:
        std::vector<std::string> names;
        std::vector<int> ordering;
        std::vector<bool> enabled;

        int count() {
            return names.size();
        }
        std::string label(int index) {
            return names[ordering[index]];
        }
        bool get_enabled(int index) {
            return enabled[ordering[index]];
        }
        void set_enabled(int index, bool enable) {
            enabled[ordering[index]] = enable;
        }
        void reorder(int old_index, int new_index) {
            if (old_index < new_index - 1) {
                int tmp = ordering[old_index];
                for (int i = old_index; i < new_index - 1; ++i) {
                    ordering[i] = ordering[i + 1];
                }
                ordering[new_index - 1] = tmp;
            }
            if (old_index > new_index) {
                int tmp = ordering[old_index];
                for (int i = old_index; i > new_index; --i) {
                    ordering[i] = ordering[i - 1];
                }
                ordering[new_index] = tmp;
            }
        }
};

struct State {
    
    Model* model;

    // Button dragging
    int active_index = -1;
    int drop_point = -1;
  
    // UI info
    float content_width = 0;
    float content_height = 0;
    float scroll_x;
  
    // UI style
    const char* font_face = "regular";
    float text_size = 16.0;
    float h_spacing = 4;
    float v_padding = 4;
    float h_padding = 8;
    float border_radius = 4;
    float overshoot = 2;
    ddui::Color color_insertion_point     = ddui::rgb(0xff0000);
    ddui::Color color_background_enabled  = ddui::rgb(0x6478ff);
    ddui::Color color_background_disabled = ddui::rgb(0x404040);
    ddui::Color color_background_vacant   = ddui::rgb(0xc8c8c8);
    ddui::Color color_text_enabled        = ddui::rgb(0xffffff);
    ddui::Color color_text_disabled       = ddui::rgb(0xa0a0a0);
    float hover_highlight_opacity = 0.2;
};

void update(State* state);

}

#endif
