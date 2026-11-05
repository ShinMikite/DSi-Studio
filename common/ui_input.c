// SPDX-License-Identifier: GPL-3.0-or-later
#include "ui.h"
UiRect ui_rect(UiAction a) {
    if(a>=UI_GUIDE0 && a<=UI_GUIDE4) return (UiRect){5+(a-UI_GUIDE0)*50,42,46,25};
    if(a>=UI_TILE0 && a<=UI_TILE3) { int i=a-UI_TILE0; return (UiRect){8+(i%2)*124,34+(i/2)*47,116,43}; }
    switch(a) {
        case UI_CARD:return (UiRect){64,4,100,25};
        case UI_REFRESH:return (UiRect){172,4,76,25};
        case UI_PREV:return (UiRect){8,129,36,23};
        case UI_NEXT:return (UiRect){212,129,36,23};
        case UI_HELP:return (UiRect){128,129,76,23};
        case UI_BACKUP:return (UiRect){8,158,88,28};
        case UI_COPY:return (UiRect){104,158,144,28};
        case UI_OPEN:return (UiRect){20,74,216,36};
        case UI_HOME:return (UiRect){20,122,216,30};
        default:return (UiRect){0,0,0,0};
    }
}
bool ui_inside(UiRect r,int x,int y) { return x>=r.x && y>=r.y && x<r.x+r.w && y<r.y+r.h; }
bool ui_can_copy(const StudioGame *g,int n,int s,bool nand) { return nand && s>=0 && s<n && g[s].metadata && !g[s].installed; }
UiAction ui_library_hit(int x,int y,const StudioGame *g,int n,int s,bool nand) {
    for(UiAction a=UI_TILE0;a<=UI_HELP;a++) {
        if(!ui_inside(ui_rect(a),x,y)) continue;
        if(a>=UI_TILE0 && a<=UI_TILE3 && (s/4*4+(int)a-UI_TILE0)>=n) return UI_NONE;
        if(a==UI_COPY && !ui_can_copy(g,n,s,nand)) return UI_NONE;
        if(a==UI_BACKUP && !nand) return UI_NONE;
        if(a==UI_PREV && s<4) return UI_NONE;
        if(a==UI_NEXT && (s/4+1)*4>=n) return UI_NONE;
        return a;
    }
    return UI_NONE;
}
UiAction ui_panel_hit(UiPanel m,int x,int y) {
    if(m==UI_MESSAGE) return ui_inside((UiRect){8,158,240,28},x,y)?UI_ACCEPT:UI_NONE;
    if(m==UI_CHOICE || m==UI_HOLD) {
        if(ui_inside(ui_rect(UI_BACKUP),x,y)) return UI_CANCEL;
        if(ui_inside(ui_rect(UI_COPY),x,y)) return UI_ACCEPT;
    }
    if(m==UI_PROGRESS_CANCEL && ui_inside(ui_rect(UI_BACKUP),x,y)) return UI_CANCEL;
    return UI_NONE;
}
UiAction ui_title_hit(int x,int y) {
    if(ui_inside(ui_rect(UI_OPEN),x,y)) return UI_OPEN;
    if(ui_inside(ui_rect(UI_HOME),x,y)) return UI_HELP;
    return UI_NONE;
}
UiAction ui_help_hit(int x,int y) {
    for(UiAction a=UI_GUIDE0;a<=UI_GUIDE4;a++) if(ui_inside(ui_rect(a),x,y)) return a;
    return ui_panel_hit(UI_MESSAGE,x,y);
}
UiAction ui_touch_update(UiTouch *s,UiAction hit,bool down,bool held,bool up) {
    if(down) s->captured=hit;
    if(held) s->hover=hit;
    UiAction result=UI_NONE;
    if(up) {
        if(s->captured!=UI_NONE && s->captured==s->hover) result=s->captured;
        s->captured=s->hover=UI_NONE;
    }
    return result;
}
