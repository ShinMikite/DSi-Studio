// SPDX-License-Identifier: GPL-3.0-or-later
#include "ui.h"
#include "version.h"
#include "font.h"
#include "shop_icon.h"
#include <stdio.h>
#include <string.h>
uint16_t studio_top[256*192],studio_bottom[256*192];
#define RGB(r,g,b) (0x8000|((r)>>3)|(((g)>>3)<<5)|(((b)>>3)<<10))
#define WHITE RGB(255,255,255)
#define INK RGB(30,48,59)
#define MUTED RGB(97,114,124)
#define BLUE RGB(0,132,184)
#define DARK RGB(0,99,148)
#define PALE RGB(229,244,250)
#define LINE RGB(198,214,222)
#define GREEN RGB(36,139,106)
static UiAction pressed=UI_NONE;
void ui_set_pressed(UiAction a) { pressed=a; }
static void box(uint16_t *p,int x,int y,int w,int h,uint16_t c) {
    for(int j=y;j<y+h;j++) for(int i=x;i<x+w;i++) if(i>=0 && i<256 && j>=0 && j<192) p[j*256+i]=c;
}
static void circle(uint16_t *p,int x,int y,int r,uint16_t c) {
    for(int j=-r;j<=r;j++) for(int i=-r;i<=r;i++) if(i*i+j*j<=r*r) box(p,x+i,y+j,1,1,c);
}
static void rounded(uint16_t *p,UiRect r,uint16_t c) {
    box(p,r.x+3,r.y,r.w-6,r.h,c); box(p,r.x,r.y+3,r.w,r.h-6,c); box(p,r.x+1,r.y+1,r.w-2,r.h-2,c);
}
static void text(uint16_t *p,int x,int y,const char *s,uint16_t c,int scale,int max) {
    for(int n=0;s[n] && n<max;n++) {
        unsigned ch=(unsigned char)s[n]; if(ch<32 || ch>126) ch='?';
        for(int j=0;j<11;j++) for(int i=0;i<6;i++) if(font[ch-32][j]&(1<<i)) box(p,x+(n*6+i)*scale,y+j*scale,scale,scale,c);
    }
}
// Center visible pixels, not the font's unused rows and advance width.
static UiRect ink(const char *s,int max) {
    int left=256,right=-1,top=11,bottom=-1;
    for(int n=0;s[n] && n<max;n++) {
        unsigned ch=(unsigned char)s[n]; if(ch<32 || ch>126) ch='?';
        for(int y=0;y<11;y++) for(int x=0;x<6;x++) if(font[ch-32][y]&(1<<x)) {
            int px=n*6+x;
            if(px<left) left=px;
            if(px>right) right=px;
            if(y<top) top=y;
            if(y>bottom) bottom=y;
        }
    }
    return right<0?(UiRect){0,0,0,0}:(UiRect){left,top,right-left+1,bottom-top+1};
}
static void center(uint16_t *p,int x,int y,int w,const char *s,uint16_t c) { UiRect b=ink(s,42); text(p,x+(w-b.w)/2-b.x,y,s,c,1,42); }
static void wrap(uint16_t *p,int x,int y,const char *s,int width,int lines,uint16_t c) {
    while(*s && lines--) {
        int n=0,last=-1; while(s[n] && s[n]!='\n' && n<width) { if(s[n]==' ') last=n; n++; }
        if(n==width && s[n] && s[n]!='\n' && last>0) n=last;
        text(p,x,y,s,c,1,n); s+=n; if(*s==' ' || *s=='\n') s++; y+=13;
    }
}
static void shop(uint16_t *p,int x,int y,int scale) {
    for(int j=0;j<32;j++) for(int i=0;i<32;i++) if(shop_icon[j*32+i]&0x8000) box(p,x+i*scale,y+j*scale,scale,scale,shop_icon[j*32+i]);
}
static void icon(uint16_t *p,int x,int y,const StudioGame *g,int scale) {
    bool any=false;
    for(int j=0;j<32;j++) for(int i=0;i<32;i++) if(g->icon[j*32+i]&0x8000) { box(p,x+i*scale,y+j*scale,scale,scale,g->icon[j*32+i]); any=true; }
    if(!any) shop(p,x,y,scale);
}
static void background(uint16_t *p) { for(int y=0;y<192;y++) box(p,0,y,256,1,RGB(247-y/24,250-y/32,252)); }
static void badge(uint16_t *p,int x,int y,int radius,const char *key,uint16_t fg,uint16_t bg) {
    UiRect b=ink(key,1);
    circle(p,x,y,radius,fg); circle(p,x,y,radius-1,bg); text(p,x-(b.w-1)/2-b.x,y-(b.h-1)/2-b.y,key,fg,1,1);
}
static void prompt(uint16_t *p,int x,int y,const char *key,uint16_t fg,uint16_t bg) { badge(p,x,y,8,key,fg,bg); }
static void button_at(UiRect r,UiAction a,const char *key,const char *label,bool primary,bool enabled) {
    uint16_t fill=primary?BLUE:WHITE,fg=primary?WHITE:INK;
    if(!enabled) { fill=RGB(226,233,237); fg=RGB(125,141,150); }
    else if(pressed==a) fill=primary?DARK:PALE;
    rounded(studio_bottom,(UiRect){r.x,r.y+1,r.w,r.h},LINE);
    rounded(studio_bottom,r,enabled && primary?fill:LINE);
    rounded(studio_bottom,(UiRect){r.x+1,r.y+1,r.w-2,r.h-2},fill);
    UiRect b=ink(label,38);
    int width=b.w+(key?23:0),left=r.x+(r.w-width)/2;
    if(key) { prompt(studio_bottom,left+8,r.y+(r.h-1)/2,key,fg,fill); left+=23; }
    text(studio_bottom,left-b.x,r.y+(r.h-b.h)/2-b.y,label,fg,1,38);
}
static void button(UiAction a,const char *key,const char *label,bool primary,bool enabled) { button_at(ui_rect(a),a,key,label,primary,enabled); }
static void header(uint16_t *p,const char *title) {
    UiRect b=ink(title,42);
    box(p,0,0,256,35,WHITE); center(p,8,(35-b.h)/2-b.y,240,title,INK); box(p,0,34,256,1,LINE);
}
void ui_library(const StudioGame *g,int count,int sel,const char *source,bool nand) {
    background(studio_top); background(studio_bottom);
    box(studio_top,0,0,256,38,WHITE); shop(studio_top,6,2,1);
    text(studio_top,44,8,"DSi Studio",INK,2,10); text(studio_top,216,16,STUDIO_VERSION,MUTED,1,5); box(studio_top,0,37,256,1,LINE);
    if(count>0) {
        const StudioGame *v=&g[sel];
        rounded(studio_top,(UiRect){8,46,240,91},WHITE);
        rounded(studio_top,(UiRect){14,52,72,72},PALE); icon(studio_top,18,56,v,2);
        wrap(studio_top,94,53,v->title,24,3,INK);
        text(studio_top,94,110,v->installed?"INSTALLED":v->metadata?"DSiWARE":"NEEDS PREPARATION",v->installed?GREEN:BLUE,1,24);
        char size[25],blocks[25]; snprintf(size,sizeof(size),"%.1f MB",(double)v->size/1048576);
        snprintf(blocks,sizeof(blocks),"%lu blocks",(unsigned long)((v->size+v->public_size+v->private_size+131071)/131072));
        text(studio_top,14,143,size,INK,1,18); text(studio_top,138,143,blocks,MUTED,1,18);
        const char *hint=v->issue[0]?v->issue:v->installed?"Already on this DSi.":nand?"Select Copy to DSi to begin checks.":"Browse only. DSi access required.";
        wrap(studio_top,14,161,hint,38,2,MUTED);
    } else {
        shop(studio_top,96,51,2); center(studio_top,0,124,256,"No games found",INK);
        center(studio_top,0,147,256,"Add games to /DSiWare",MUTED); center(studio_top,0,163,256,"then select Scan.",MUTED);
    }
    text(studio_bottom,9,11,"LIBRARY",INK,1,10);
    button(UI_CARD,"L","Switch card",false,true); button(UI_REFRESH,"X","Scan",false,true);
    int page=sel/4;
    for(int i=0;i<4;i++) {
        int k=page*4+i; UiRect r=ui_rect(UI_TILE0+i); bool active=k==sel && k<count;
        rounded(studio_bottom,r,active?BLUE:LINE);
        rounded(studio_bottom,(UiRect){r.x+1,r.y+1,r.w-2,r.h-2},active?PALE:WHITE);
        if(k<count) {
            if(pressed==(UiAction)(UI_TILE0+i)) rounded(studio_bottom,(UiRect){r.x+2,r.y+2,r.w-4,r.h-4},RGB(202,231,243));
            icon(studio_bottom,r.x+5,r.y+5,&g[k],1); wrap(studio_bottom,r.x+42,r.y+4,g[k].title,11,2,INK);
            text(studio_bottom,r.x+42,r.y+29,g[k].installed?"Installed":g[k].metadata?"DSiWare":"Check file",g[k].installed?GREEN:MUTED,1,11);
        }
    }
    button(UI_PREV,NULL,"<",false,sel>=4); button(UI_NEXT,NULL,">",false,(page+1)*4<count);
    char pages[32]; snprintf(pages,sizeof(pages),"%d / %d",count?page+1:0,(count+3)/4); center(studio_bottom,48,135,72,pages,MUTED);
    button(UI_HELP,NULL,"Help",false,true); button(UI_BACKUP,"Y","Backup",false,nand);
    button(UI_COPY,"A",count && g[sel].installed?"Installed":"Copy to DSi",true,ui_can_copy(g,count,sel,nand));
    text(studio_top,176,25,source,MUTED,1,12);
}
void ui_title(void) {
    background(studio_top); background(studio_bottom); box(studio_top,0,0,256,4,BLUE); shop(studio_top,96,25,2);
    text(studio_top,68,103,"DSi Studio",INK,2,10); center(studio_top,0,136,256,"DSiWARE MANAGER",MUTED);
    box(studio_top,72,161,112,1,LINE); center(studio_top,0,173,256,"Version " STUDIO_VERSION,MUTED);
    header(studio_bottom,"DSi Studio");
    button(UI_OPEN,"A","Open library",true,true); button_at(ui_rect(UI_HOME),UI_HELP,NULL,"How to use Studio",false,true);
}
void ui_help(int page) {
    background(studio_bottom); header(studio_bottom,"HOW TO USE STUDIO");
    const char *tabs[]={"Install","Keys","Backup","Safety","Restore"};
    for(int i=0;i<5;i++) button(UI_GUIDE0+i,NULL,tabs[i],page==i,true);
    rounded(studio_bottom,(UiRect){8,73,240,77},WHITE);
    if(page==1) {
        const char *keys[]={"A","Y","X","L","B"};
        const char *labels[]={"Copy game - hold to confirm","Back up / check backup","Scan for games","Switch flashcard / DSi SD","Return to title screen"};
        for(int i=0;i<5;i++) {
            int y=81+i*15;
            badge(studio_bottom,23,y,6,keys[i],INK,WHITE);
            UiRect b=ink(labels[i],34);
            text(studio_bottom,41,y-(b.h-1)/2-b.y,labels[i],INK,1,34);
        }
    } else {
        const char *body=page==0?
            "1. Copy ROM + .tmd to /DSiWare.\n2. Back up this DSi when prompted.\n3. Select a game; tap Copy to DSi.\n4. Let Studio check your files.\n5. Hold Copy to confirm.\n6. Restart when copying is done.":page==2?
            "A backup is required for each DSi.\nOther consoles' backups stay saved.\nKeep a copy on your PC.\nFull check: once per Studio session.\nSwitching cards starts a new check.\nBackup checks again on demand.":
            page==3?"Copying changes internal storage.\nA failed write can stop your DSi\nfrom starting. Backups cannot\nguarantee recovery. Keep the charger\nconnected. Power off before removing\na card. Never interrupt copying.":
            "Recovery needs DSi ntrboot or\nhardware access to NAND.\nStudio cannot restore a backup.\nNormal flashcard boot isn't enough.\nRead the recovery guide:\ngithub.com/ShinMikite/DSi-Studio";
        wrap(studio_bottom,16,77,body,37,6,INK);
    }
    button_at((UiRect){8,158,240,28},UI_ACCEPT,"A","Done",true,true);
}
void ui_panel(const char *title,const char *body,UiPanel mode,int percent) {
    background(studio_bottom); header(studio_bottom,title); rounded(studio_bottom,(UiRect){8,42,240,108},WHITE);
    bool progress=mode==UI_PROGRESS || mode==UI_PROGRESS_CANCEL;
    wrap(studio_bottom,16,48,body,37,progress?6:7,INK);
    if(progress && percent>=0) {
        char value[8]; snprintf(value,sizeof(value),"%d%%",percent>100?100:percent); text(studio_bottom,211,125,value,BLUE,1,6);
        rounded(studio_bottom,(UiRect){16,141,224,5},PALE); box(studio_bottom,16,142,224*(percent>100?100:percent)/100,3,BLUE);
    }
    if(mode==UI_MESSAGE) button_at((UiRect){8,158,240,28},UI_ACCEPT,"A","Done",true,true);
    else if(mode==UI_CHOICE || mode==UI_HOLD) {
        button_at(ui_rect(UI_BACKUP),UI_CANCEL,"B","Back",false,true);
        button_at(ui_rect(UI_COPY),UI_ACCEPT,"A",mode==UI_CHOICE?"Back up":"Hold to copy",true,true);
        if(mode==UI_HOLD && percent>0) box(studio_bottom,108,181,136*(percent>100?100:percent)/100,3,WHITE);
    } else if(mode==UI_PROGRESS_CANCEL) {
        button_at(ui_rect(UI_BACKUP),UI_CANCEL,"B","Cancel",false,true); text(studio_bottom,111,167,"Keep card inserted",MUTED,1,23);
    } else center(studio_bottom,0,167,256,percent>=0?"Keep the console powered on":"Please wait",MUTED);
}
