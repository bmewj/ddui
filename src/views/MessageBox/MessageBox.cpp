//
//  MessageBox.cpp
//  ddui
//
//  Created by Bartholomew Joyce on 06/08/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include "MessageBox.hpp"

namespace MessageBox {

static void update_content(MessageBoxState* state, Context ctx);

MessageBoxState::MessageBoxState() {
    overlay_box.max_width = 300;
    overlay_box.max_height = 150;
}

void open(MessageBoxState* state) {
    OverlayBox::open(&state->overlay_box);
}

void close(MessageBoxState* state) {
    OverlayBox::close(&state->overlay_box);
}

void update(MessageBoxState* state) {
    OverlayBox::update(&state->overlay_box, [state](Context ctx) {
        update_content(state, ctx);
    });
}

constexpr auto PADDING = 10;
constexpr auto TITLE_SIZE = 20;
static auto TEXT_COLOR = nvgRGB(0xcc, 0xcc, 0xcc);

void update_content(MessageBoxState* state, Context ctx) {

    auto y = 0;
    
    // Title
    nvgFontFace(ctx.vg, "regular");
    nvgFontSize(ctx.vg, TITLE_SIZE);
    nvgFillColor(ctx.vg, TEXT_COLOR);
    nvgTextAlign(ctx.vg, NVG_ALIGN_TOP | NVG_ALIGN_CENTER);
    nvgText(ctx.vg, ctx.width / 2, y + PADDING, state->title.c_str(), NULL);
    nvgTextAlign(ctx.vg, NVG_ALIGN_LEFT);
    y += TITLE_SIZE + 2 * PADDING;

    // Message
    
    
    if (mouse_hit(ctx, 0, 0, ctx.width, ctx.height)) {
        ctx.mouse->accepted = true;
    }

}

}
