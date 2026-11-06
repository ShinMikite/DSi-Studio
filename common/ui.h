// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "catalog.h"
#include "storage.h"
extern uint16_t studio_top[256*192],studio_bottom[256*192];
typedef enum { UI_NONE,UI_TILE0,UI_TILE1,UI_TILE2,UI_TILE3,UI_COPY,UI_BACKUP,UI_CARD,UI_REFRESH,UI_PREV,UI_NEXT,UI_HELP,UI_ACCEPT,UI_CANCEL,UI_OPEN,UI_HOME,UI_GUIDE0,UI_GUIDE1,UI_GUIDE2,UI_GUIDE3,UI_GUIDE4 } UiAction;
typedef enum { UI_MESSAGE,UI_CHOICE,UI_HOLD,UI_PROGRESS,UI_PROGRESS_CANCEL } UiPanel;
typedef struct { int x,y,w,h; } UiRect;
typedef struct { UiAction captured,hover; } UiTouch;
UiRect ui_rect(UiAction action);
bool ui_inside(UiRect rect,int x,int y);
bool ui_can_copy(const StudioGame *games,int count,int selected,bool nand);
UiAction ui_library_hit(int x,int y,const StudioGame *games,int count,int selected,bool nand);
UiAction ui_panel_hit(UiPanel mode,int x,int y);
UiAction ui_title_hit(int x,int y);
UiAction ui_help_hit(int x,int y);
UiAction ui_touch_update(UiTouch *state,UiAction hit,bool down,bool held,bool up);
void ui_set_pressed(UiAction action);
void ui_library(const StudioGame *games,int count,int selected,const char *source,bool nand,const StudioStorage *storage);
void ui_panel(const char *title,const char *body,UiPanel mode,int progress);
void ui_title(void);
void ui_help(int page);
