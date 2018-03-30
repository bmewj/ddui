//
//  ContextMenu.macos.mm
//  ddui
//
//  Created by Bartholomew Joyce on 22/03/2018.
//  Copyright © 2018 Bartholomew Joyce All rights reserved.
//

#include <Cocoa/Cocoa.h>
#include "ContextMenu.hpp"
#include <ddui/Context>
#include <ddui/util/entypo>
#include <ddui/views/ScrollArea>
#include <nanovg.h>

// The native window is defined in app.macos.mm
extern NSWindow* native_window;

namespace ContextMenu {

static struct {
    std::vector<NSMenuItem*> items;
    void* identifier = NULL;
    int action = -1;
} state;

int process_action(Context ctx, void* identifier) {
    if (state.identifier == identifier) {
        int action = state.action;
        state.action = -1;
        state.identifier = NULL;
        return action;
    }
    
    return -1;
}

void show(Context ctx, void* identifier, std::vector<Item> items) {

    CGFloat contentHeight = [native_window contentRectForFrameRect: native_window.frame].size.height;

    NSPoint point;
    point.x = ctx.mouse->x;
    point.y = contentHeight - ctx.mouse->y;

    NSEvent* theEvent = [NSEvent mouseEventWithType:NSEventTypeLeftMouseDown location:point modifierFlags:0 timestamp:0 windowNumber:[native_window windowNumber] context:nil eventNumber:0 clickCount:0 pressure:0.0];

    NSMenu *theMenu = [[NSMenu alloc] initWithTitle:@"Contextual Menu"];
    
    state.items.clear();
    state.identifier = identifier;
    state.action = -1;
    
    int count = 0;
    for (auto& item : items) {
        NSMenuItem* theItem;
    
        if (item.is_separator) {
            theItem = [NSMenuItem separatorItem];
        } else if (item.enabled) {
            theItem = [[NSMenuItem alloc] initWithTitle:[NSString stringWithUTF8String:item.label.c_str()]
                                                 action:@selector(menuItemClick:)
                                          keyEquivalent:@""];
        } else {
            theItem = [[NSMenuItem alloc] initWithTitle:[NSString stringWithUTF8String:item.label.c_str()] action:nil keyEquivalent:@""];
        }

        if (!item.is_separator && item.checked) [theItem setState:NSOnState];
        
        state.items.push_back(theItem);
        [theMenu insertItem:theItem atIndex:count++];
    }

    [NSMenu popUpContextMenu:theMenu withEvent:theEvent forView:[native_window contentView]];
    
    if (ctx.mouse->pressed || ctx.mouse->pressed_secondary) {
        ctx.mouse->accepted = true;
    }
    
}

}

// Modify the GLFW window

typedef struct _GLFWwindow _GLFWwindow;
@interface GLFWWindowDelegate : NSObject
{
    _GLFWwindow* window;
}

- (instancetype)initWithGlfwWindow:(_GLFWwindow *)initWindow;

@end

@interface GLFWWindowDelegate (GLFWWindowDelegateContextMenuAdditions)

- (void)menuItemClick:(id)sender;

@end

@implementation GLFWWindowDelegate (GLFWWindowDelegateContextMenuAdditions)

- (void)menuItemClick:(id)sender {
    using namespace ContextMenu;

    for (int i = 0; i < state.items.size(); ++i) {
        if (sender == state.items[i]) {
            state.action = i;
            return;
        }
    }
    
    state.action = -1;
    
}

@end
