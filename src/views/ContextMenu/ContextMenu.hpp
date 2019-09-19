//
//  ContextMenu.hpp
//  ddui
//
//  Created by Bartholomew Joyce on 19/09/2019.
//  Copyright © 2019 Bartholomew Joyce All rights reserved.
//

#ifndef ddui_ContextMenu_hpp
#define ddui_ContextMenu_hpp

#include <ddui/core>
#include <functional>
#include <ddui/views/Menu>

struct ContextMenu {

    struct Handler {
        Handler(const std::function<void(MenuBuilder::Menu&)>& handler_fn);
        Handler(Handler&&) = delete;
        Handler(const Handler&) = delete;
        ~Handler();

        const std::function<void(MenuBuilder::Menu&)>& handler_fn;
        const Handler* parent_handler;
        bool should_activate;
    };

    static void update(const std::function<void()>& inner_update);
    static void open();

};

#endif
