#include "ui.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    StudioGame g[5]={0}; for(int i=0;i<5;i++) g[i].metadata=true;
    assert(ui_library_hit(120,170,g,5,0,true)==UI_COPY);
    assert(ui_library_hit(120,170,g,5,0,false)==UI_NONE);
    g[0].installed=true; assert(ui_library_hit(120,170,g,5,0,true)==UI_NONE); g[0].installed=false;
    g[0].metadata=false; assert(ui_library_hit(120,170,g,5,0,true)==UI_NONE); g[0].metadata=true;
    assert(ui_library_hit(220,140,g,5,0,true)==UI_NEXT);
    assert(ui_library_hit(220,140,g,5,4,true)==UI_NONE);
    assert(ui_library_hit(12,140,g,5,0,true)==UI_NONE);
    assert(ui_library_hit(12,140,g,5,4,true)==UI_PREV);
    assert(ui_library_hit(150,50,g,5,4,true)==UI_NONE);
    assert(ui_library_hit(120,170,g,0,0,true)==UI_NONE);
    assert(ui_panel_hit(UI_PROGRESS,120,170)==UI_NONE);
    assert(ui_panel_hit(UI_HOLD,40,170)==UI_CANCEL);
    assert(ui_panel_hit(UI_HOLD,120,170)==UI_ACCEPT);
    assert(ui_panel_hit(UI_HOLD,99,170)==UI_NONE);
    assert(ui_title_hit(128,96)==UI_OPEN);
    assert(ui_title_hit(128,140)==UI_HELP);
    for(int page=0;page<5;page++) assert(ui_help_hit(20+50*page,50)==(UiAction)(UI_GUIDE0+page));
    assert(ui_help_hit(128,170)==UI_ACCEPT);
    assert(ui_help_hit(128,120)==UI_NONE);
    UiTouch t={0};
    assert(ui_touch_update(&t,UI_COPY,true,true,false)==UI_NONE);
    assert(ui_touch_update(&t,UI_NONE,false,false,true)==UI_COPY);
    ui_touch_update(&t,UI_COPY,true,true,false);
    ui_touch_update(&t,UI_NONE,false,true,false);
    assert(ui_touch_update(&t,UI_NONE,false,false,true)==UI_NONE);
    ui_touch_update(&t,UI_NONE,true,true,false);
    ui_touch_update(&t,UI_COPY,false,true,false);
    assert(ui_touch_update(&t,UI_NONE,false,false,true)==UI_NONE);
    ui_touch_update(&t,UI_BACKUP,true,true,false);
    ui_touch_update(&t,UI_COPY,false,true,false);
    assert(ui_touch_update(&t,UI_NONE,false,false,true)==UI_NONE);
    puts("Touch controls: disabled actions, paging, panel bounds, drag-out and drag-in checks passed.");
}
