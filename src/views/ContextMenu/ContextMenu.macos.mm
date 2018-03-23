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

@interface menuManager : NSObject {
  struct mapTable *items;
//  BOOL hasQuit;
//  BOOL hasPreferences;
//  BOOL hasAbout;
}
//@property (strong) NSMenuItem *quitItem;
//@property (strong) NSMenuItem *preferencesItem;
//@property (strong) NSMenuItem *aboutItem;
// NSMenuValidation is only informal
- (BOOL)validateMenuItem:(NSMenuItem *)item;
- (NSMenu *)makeMenubar;
@end

@implementation menuManager

- (id)init
{
  self = [super init];
  if (self) {
    self->items = newMap();
  }
  return self;
}

- (void)dealloc
{
  mapWalk(self->items, mapItemReleaser);
  mapReset(self->items);
  mapDestroy(self->items);
  uninitMenus();
  [super dealloc];
}

- (IBAction)onClicked:(id)sender
{
  uiMenuItem *item;

  item = (uiMenuItem *) mapGet(self->items, sender);
  if (item->type == typeCheckbox)
    uiMenuItemSetChecked(item, !uiMenuItemChecked(item));
  // use the key window as the source of the menu event; it's the active window
  (*(item->onClicked))(item, windowFromNSWindow([realNSApp() keyWindow]), item->onClickedData);
}

- (IBAction)onQuitClicked:(id)sender
{
  if (shouldQuit())
    uiQuit();
}

- (void)register:(NSMenuItem *)item to:(uiMenuItem *)smi
{
  switch (smi->type) {
  case typeQuit:
    if (self->hasQuit)
      userbug("You can't have multiple Quit menu items in one program.");
    self->hasQuit = YES;
    break;
  case typePreferences:
    if (self->hasPreferences)
      userbug("You can't have multiple Preferences menu items in one program.");
    self->hasPreferences = YES;
    break;
  case typeAbout:
    if (self->hasAbout)
      userbug("You can't have multiple About menu items in one program.");
    self->hasAbout = YES;
    break;
  }
  mapSet(self->items, item, smi);
}

// on OS X there are two ways to handle menu items being enabled or disabled: automatically and manually
// unfortunately, the application menu requires automatic menu handling for the Hide, Hide Others, and Show All items to work correctly
// therefore, we have to handle enabling of the other options ourselves
- (BOOL)validateMenuItem:(NSMenuItem *)item
{
  uiMenuItem *smi;

  // disable the special items if they aren't present
  if (item == self.quitItem && !self->hasQuit)
    return NO;
  if (item == self.preferencesItem && !self->hasPreferences)
    return NO;
  if (item == self.aboutItem && !self->hasAbout)
    return NO;
  // then poll the item's enabled/disabled state
  smi = (uiMenuItem *) mapGet(self->items, item);
  return !smi->disabled;
}

// Cocoa constructs the default application menu by hand for each program; that's what MainMenu.[nx]ib does
- (void)buildApplicationMenu:(NSMenu *)menubar
{
  NSString *appName;
  NSMenuItem *appMenuItem;
  NSMenu *appMenu;
  NSMenuItem *item;
  NSString *title;
  NSMenu *servicesMenu;

  // note: no need to call setAppleMenu: on this anymore; see https://developer.apple.com/library/mac/releasenotes/AppKit/RN-AppKitOlderNotes/#X10_6Notes
  appName = [[NSProcessInfo processInfo] processName];
  appMenuItem = [[[NSMenuItem alloc] initWithTitle:appName action:NULL keyEquivalent:@""] autorelease];
  appMenu = [[[NSMenu alloc] initWithTitle:appName] autorelease];
  [appMenuItem setSubmenu:appMenu];
  [menubar addItem:appMenuItem];

  // first is About
  title = [@"About " stringByAppendingString:appName];
  item = [[[NSMenuItem alloc] initWithTitle:title action:@selector(onClicked:) keyEquivalent:@""] autorelease];
  [item setTarget:self];
  [appMenu addItem:item];
  self.aboutItem = item;

  [appMenu addItem:[NSMenuItem separatorItem]];

  // next is Preferences
  item = [[[NSMenuItem alloc] initWithTitle:@"Preferences…" action:@selector(onClicked:) keyEquivalent:@","] autorelease];
  [item setTarget:self];
  [appMenu addItem:item];
  self.preferencesItem = item;

  [appMenu addItem:[NSMenuItem separatorItem]];

  // next is Services
  item = [[[NSMenuItem alloc] initWithTitle:@"Services" action:NULL keyEquivalent:@""] autorelease];
  servicesMenu = [[[NSMenu alloc] initWithTitle:@"Services"] autorelease];
  [item setSubmenu:servicesMenu];
  [realNSApp() setServicesMenu:servicesMenu];
  [appMenu addItem:item];

  [appMenu addItem:[NSMenuItem separatorItem]];

  // next are the three hiding options
  title = [@"Hide " stringByAppendingString:appName];
  item = [[[NSMenuItem alloc] initWithTitle:title action:@selector(hide:) keyEquivalent:@"h"] autorelease];
  // the .xib file says they go to -1 ("First Responder", which sounds wrong...)
  // to do that, we simply leave the target as nil
  [appMenu addItem:item];
  item = [[[NSMenuItem alloc] initWithTitle:@"Hide Others" action:@selector(hideOtherApplications:) keyEquivalent:@"h"] autorelease];
  [item setKeyEquivalentModifierMask:(NSAlternateKeyMask | NSCommandKeyMask)];
  [appMenu addItem:item];
  item = [[[NSMenuItem alloc] initWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""] autorelease];
  [appMenu addItem:item];

  [appMenu addItem:[NSMenuItem separatorItem]];

  // and finally Quit
  // DON'T use @selector(terminate:) as the action; we handle termination ourselves
  title = [@"Quit " stringByAppendingString:appName];
  item = [[[NSMenuItem alloc] initWithTitle:title action:@selector(onQuitClicked:) keyEquivalent:@"q"] autorelease];
  [item setTarget:self];
  [appMenu addItem:item];
  self.quitItem = item;
}

- (NSMenu *)makeMenubar
{
  NSMenu *menubar;

  menubar = [[[NSMenu alloc] initWithTitle:@""] autorelease];
  [self buildApplicationMenu:menubar];
  return menubar;
}

@end

namespace ContextMenu {

ContextMenuState::ContextMenuState() {
    open = false;
    identifier = NULL;
}

static constexpr int ITEM_HEIGHT = 28;
static constexpr int ITEM_FONT_SIZE = 16;
static constexpr int CHECK_FONT_SIZE = 32;
static constexpr const char* ITEM_FONT_FACE = "regular";
static NVGcolor ITEM_TEXT_COLOR = nvgRGB(0, 0, 0);
static NVGcolor ITEM_HOVER_COLOR = nvgRGB(200, 200, 200);
static NVGcolor ITEM_PRESS_COLOR = nvgRGB(150, 150, 150);
static NVGcolor BG_COLOR = nvgRGB(255, 255, 255);
static constexpr int PADDING_TOP = 4;
static constexpr int PADDING_BOTTOM = 4;

static constexpr int PADDING_HORIZONTAL = 9;
static constexpr int PADDING_LEFT = 32;
static constexpr int PADDING_RIGHT = 16;

void update(ContextMenuState* state, Context ctx, std::function<void(Context)> inner_update) {
    // Step 1. Process mouse input
    void* identifier = state->identifier;;
    int menu_width, menu_height;
    if (state->open) {
        menu_height = PADDING_TOP + ITEM_HEIGHT * state->items.size() + PADDING_BOTTOM;

        nvgFontFace(ctx.vg, ITEM_FONT_FACE);
        nvgFontSize(ctx.vg, ITEM_FONT_SIZE);

        float bounds[4];
        int max_text_width = 0;
        for (auto& item : state->items) {
          nvgTextBounds(ctx.vg, 0, 0, item.label.c_str(), 0, bounds);
          int width = (int)(bounds[2] - bounds[0]);
          if (width > max_text_width) {
              max_text_width = width;
          }
        }
      
        menu_width = PADDING_LEFT + max_text_width + PADDING_RIGHT;

        // Handle context menu dismissal
        if (mouse_hit(ctx, 0, 0, ctx.width, ctx.height) &&
            !mouse_hit(ctx, state->x, state->y, menu_width, menu_height)) {
            ctx.mouse->accepted = true;
            state->open = false;
            state->action = -1;
        }

        // Handle item press
        int y = state->y + PADDING_TOP;
        for (int i = 0; i < state->items.size(); ++i) {
            if (mouse_hit(ctx, state->x, y, menu_width, ITEM_HEIGHT)) {
                ctx.mouse->accepted = true;
                state->action_pressing = i;
                break;
            }
          
            y += ITEM_HEIGHT;
        }

        // Handle item release
        if (!ctx.mouse->pressed && state->action_pressing != -1) {
            state->open = false;

            int y = state->y + PADDING_TOP + state->action_pressing * ITEM_HEIGHT;
            if (mouse_over(ctx, state->x, y, menu_width, ITEM_HEIGHT)) {
                state->action = state->action_pressing;
            } else {
                state->action = -1;
            }
        }
      
        // Handle border clicking
        if (mouse_hit(ctx, state->x, state->y, menu_width, menu_height)) {
            ctx.mouse->accepted = true;
            state->action_pressing = -1;
        }
      
    }

    // Step 2. Draw child content
    auto child_ctx = ctx;
    child_ctx.context_menu_state = state;
    inner_update(child_ctx);

    // Step 3. Draw context menu
    if (state->open && identifier == state->identifier) {

        int width = ctx.width - state->x;
        if (width > menu_width) {
            width = menu_width;
        }
      
        int height = ctx.height - state->y;
        if (height > menu_height) {
            height = menu_height;
        }
      
        auto child_ctx = child_context(ctx, state->x, state->y, width, height);
        ScrollArea::update(&state->scroll_area_state, child_ctx, menu_width, menu_height, [&](Context ctx) {
        
            // Background
            nvgBeginPath(ctx.vg);
            nvgFillColor(ctx.vg, nvgRGB(255, 255, 255));
            nvgRect(ctx.vg, 0, 0, ctx.width, ctx.height);
            nvgFill(ctx.vg);
          
            // Items
            float ascender, descender, line_height;
            nvgFontFace(ctx.vg, ITEM_FONT_FACE);
            nvgFontSize(ctx.vg, ITEM_FONT_SIZE);
            nvgTextMetrics(ctx.vg, &ascender, &descender, &line_height);
          
            int y = PADDING_TOP;
            int text_y = (ITEM_HEIGHT - (int)line_height) / 2 + (int)ascender;
            int text_x = PADDING_LEFT;
            for (int i = 0; i < state->items.size(); ++i) {
                int hover_state = 0;

                if (mouse_over(ctx, 0, y, menu_width, ITEM_HEIGHT)) {
                    hover_state = 1;
                    *ctx.cursor = CURSOR_POINTING_HAND;
                }
              
                if (state->action_pressing == i) {
                    hover_state = 2;
                  
                    if (ctx.mouse->x < state->x || ctx.mouse->x > state->x + menu_width ||
                        ctx.mouse->y < y || ctx.mouse->y > y + ITEM_HEIGHT) {
                        --hover_state;
                    }
                }

                if (hover_state > 0) {
                    nvgBeginPath(ctx.vg);
                    nvgFillColor(ctx.vg, hover_state == 2 ? ITEM_PRESS_COLOR : ITEM_HOVER_COLOR);
                    nvgRect(ctx.vg, state->x, y, menu_width, ITEM_HEIGHT);
                    nvgFill(ctx.vg);
                }
              
                nvgFillColor(ctx.vg, ITEM_TEXT_COLOR);
                nvgText(ctx.vg, text_x, y + text_y, state->items[i].label.c_str(), 0);
            
                y += ITEM_HEIGHT;
            }

            nvgFontSize(ctx.vg, CHECK_FONT_SIZE);
            nvgFontFace(ctx.vg, "entypo");
            nvgFillColor(ctx.vg, ITEM_TEXT_COLOR);
            nvgTextMetrics(ctx.vg, &ascender, &descender, &line_height);

            y = PADDING_TOP + (ascender + ITEM_HEIGHT) / 2 - 4;
            for (int i = 0; i < state->items.size(); ++i) {
                if (state->items[i].checked) {
                    nvgText(ctx.vg, PADDING_HORIZONTAL, y, entypo::CHECK_MARK, NULL);
                }

                y += ITEM_HEIGHT;
            }

        });
        nvgRestore(ctx.vg);
          
    }
}

int process_action(Context ctx, void* identifier) {
    auto state = ctx.context_menu_state;

    if (!state->open && state->identifier == identifier) {
        state->identifier = NULL;
        return state->action;
    }

    return -1;
}

void show(Context ctx, void* identifier, int x, int y, std::vector<Item> items) {
    auto state = ctx.context_menu_state;
    
    state->open = true;
    state->identifier = identifier;
    state->action_pressing = -1;
    state->action = -1;
    state->x = ctx.x + x;
    state->y = ctx.y + y;
    state->items = std::move(items);

    *ctx.must_repaint = true;
}

}
