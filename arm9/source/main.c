// SPDX-License-Identifier: GPL-3.0-or-later
#include "studio.h"
#include "crypto.h"
#include "audio.h"
#include "game_files.h"
#include <fat.h>
#include <nds/arm9/sdmmc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

StudioGame studio_games[STUDIO_MAX_GAMES];
int studio_count=0,studio_selected=0,studio_region=-1;
bool studio_nand=false,studio_crypto=false;
uint32_t studio_console_id[2]={0};
char studio_root[8]="fat:/";
static int top_bg,bottom_bg;
static StudioStorage storage;

void studio_present(void) {
    swiWaitForVBlank();
    DC_FlushRange(studio_top,sizeof(studio_top)); DC_FlushRange(studio_bottom,sizeof(studio_bottom));
    dmaCopy(studio_top,bgGetGfxPtr(top_bg),sizeof(studio_top));
    dmaCopy(studio_bottom,bgGetGfxPtr(bottom_bg),sizeof(studio_bottom));
}
static void library(void) {
    ui_library(studio_games,studio_count,studio_selected,!strcmp(studio_root,"fat:/")?"FLASHCARD SD":"DSi SD CARD",studio_nand&&studio_crypto,&storage);
}
static void release_keys(void) { do { swiWaitForVBlank(); scanKeys(); } while(keysHeld()&(KEY_A|KEY_B|KEY_TOUCH)); }
static UiAction touch_panel(UiTouch *state,UiPanel mode) {
    UiAction hit=UI_NONE;
    if(keysHeld()&KEY_TOUCH) { touchPosition t; touchRead(&t); hit=ui_panel_hit(mode,t.px,t.py); }
    UiAction action=ui_touch_update(state,hit,keysDown()&KEY_TOUCH,keysHeld()&KEY_TOUCH,keysUp()&KEY_TOUCH);
    ui_set_pressed(state->captured==state->hover?state->hover:UI_NONE);
    return action;
}
void studio_notice(const char *title,const char *body) {
    if(strstr(title,"SAVED") || strstr(title,"VERIFIED") || strstr(title,"ADDED")) studio_sound(AUDIO_SUCCESS);
    else if(strstr(title,"NOT") || strstr(title,"FAILED") || strstr(title,"CANNOT") || strstr(title,"STOPPED")) studio_sound(AUDIO_ERROR);
    release_keys(); UiTouch touch={0};
    for(;;) {
        scanKeys(); UiAction action=touch_panel(&touch,UI_MESSAGE);
        if((keysDown()&(KEY_A|KEY_B)) || action==UI_ACCEPT) break;
        ui_panel(title,body,UI_MESSAGE,-1); studio_present();
    }
    ui_set_pressed(UI_NONE); release_keys();
}
bool studio_choice(const char *title,const char *body) {
    release_keys(); UiTouch touch={0};
    for(;;) {
        scanKeys(); UiAction action=touch_panel(&touch,UI_CHOICE);
        if((keysDown()&KEY_B) || action==UI_CANCEL) { ui_set_pressed(UI_NONE); studio_sound(AUDIO_BACK); return false; }
        if((keysDown()&KEY_A) || action==UI_ACCEPT) { ui_set_pressed(UI_NONE); studio_sound(AUDIO_ACCEPT); return true; }
        ui_panel(title,body,UI_CHOICE,-1); studio_present();
    }
}
bool studio_confirm(const char *title,const char *body) {
    release_keys(); int held=0; UiTouch touch={0};
    while(1) {
        scanKeys(); UiAction action=touch_panel(&touch,UI_HOLD);
        if((keysDown()&KEY_B) || action==UI_CANCEL) { ui_set_pressed(UI_NONE); studio_sound(AUDIO_BACK); return false; }
        bool touching=(keysHeld()&KEY_TOUCH) && touch.captured==UI_ACCEPT && touch.hover==UI_ACCEPT;
        if((keysHeld()&KEY_A) || touching) held++; else held=0;
        ui_set_pressed(held?UI_ACCEPT:touch.hover);
        ui_panel(title,body,UI_HOLD,held*100/90); studio_present();
        if(held>=90) { ui_set_pressed(UI_NONE); studio_sound(AUDIO_ACCEPT); return true; }
    }
}
void studio_progress(const char *s,int percent) {
    const char *title=!strncmp(s,"Checking",8) || !strncmp(s,"Verifying Nintendo",18)?"CHECK GAME":
                      !strncmp(s,"Verifying",9)?"VERIFY COPY":"COPY TO DSi MENU";
    studio_music(false); ui_set_pressed(UI_NONE); ui_panel(title,s,UI_PROGRESS,percent); studio_present();
}
uint64_t studio_size(const char *p) { struct stat s; return stat(p,&s)==0 && S_ISREG(s.st_mode)?s.st_size:0; }
bool studio_read(const char *p,void *out,size_t n) {
    FILE *f=fopen(p,"rb"); if(!f) return false;
    bool ok=fread(out,1,n,f)==n; if(fclose(f)) ok=false; return ok;
}
static void banner(StudioGame *g,const uint8_t *header) {
    uint32_t off=studio_le32(header+0x68); uint8_t b[0x840];
    if(!off || (uint64_t)off+sizeof(b)>g->size) return;
    FILE *f=fopen(g->path,"rb"); if(!f) return;
    bool ok=fseek(f,off,SEEK_SET)==0 && fread(b,1,sizeof(b),f)==sizeof(b); fclose(f); if(!ok) return;
    unsigned lang=PersonalData->language; if(lang>5) lang=1;
    const uint8_t *t=b+0x240+lang*256; if(t[0]==0 && t[1]==0) t=b+0x340;
    for(int i=0;i<128;i++) {
        unsigned c=t[i*2]|t[i*2+1]<<8;
        g->title[i]=c==10?' ':c>=32&&c<127?c:c?'?':0; if(!c) break;
    }
    g->title[128]=0;
    for(int y=0;y<32;y++) for(int x=0;x<32;x++) {
        int pos=((y/8)*4+x/8)*32+(y%8)*4+(x%8)/2;
        int c=(b[0x20+pos]>>((x%2)*4))&15;
        g->icon[y*32+x]=c?(0x8000|b[0x220+c*2]|b[0x221+c*2]<<8):0;
    }
}
static void scan_dir(const char *path,int depth) {
    if(depth>8 || studio_count>=STUDIO_MAX_GAMES) return;
    DIR *d=opendir(path); if(!d) return;
    struct dirent *e;
    while((e=readdir(d)) && studio_count<STUDIO_MAX_GAMES) {
        if(e->d_name[0]=='.') continue;
        char full[STUDIO_PATH]; int n=snprintf(full,sizeof(full),"%s/%s",path,e->d_name);
        if(n<0 || n>=(int)sizeof(full)) continue;
        struct stat st; if(stat(full,&st)) continue;
        if(S_ISDIR(st.st_mode)) { scan_dir(full,depth+1); continue; }
        if(!S_ISREG(st.st_mode) || !studio_game_file(full)) continue;
        uint8_t h[0x240]; StudioGame *g=&studio_games[studio_count]; memset(g,0,sizeof(*g));
        if(!studio_read(full,h,sizeof(h)) || !studio_header(h,sizeof(h),st.st_size,g)) continue;
        snprintf(g->path,sizeof(g->path),"%s",full); banner(g,h);
        uint8_t t[520]; studio_game_metadata(g,t);
        if(studio_nand) {
            char installed[80]; snprintf(installed,sizeof(installed),"nand:/title/00030004/%08lx",(unsigned long)g->id);
            g->installed=access(installed,F_OK)==0;
        }
        studio_count++;
    }
    closedir(d);
}
static int compare(const void *a,const void *b) { return strcmp(((StudioGame*)a)->title,((StudioGame*)b)->title); }
void studio_scan(void) {
    studio_count=studio_selected=0;
    char dir[32]; snprintf(dir,sizeof(dir),"%sDSiWare",studio_root);
    ui_panel("LIBRARY","Reading games on your card...",UI_PROGRESS,-1); studio_present();
    scan_dir(dir,0); qsort(studio_games,studio_count,sizeof(StudioGame),compare);
    studio_storage_read(&storage);
}
static void setup_storage(int argc,char **argv) {
    bool sd=fatInitDefault(); (void)sd;
    if(isDSiMode()) {
        strcpy(studio_root,"sd:/");
        studio_nand=nandInit(true);
        if(studio_nand) {
            FILE *f=fopen("nand:/sys/HWINFO_S.dat","rb");
            if(f) { if(fseek(f,0x90,SEEK_SET)==0) studio_region=fgetc(f); fclose(f); }
            if(studio_region<0 || studio_region>5) { studio_region=-1; studio_nand=false; }
        }
        uint32_t ids[2];
        for(int j=0;j<2;j++) {
            int frames=0;
            while(!fifoCheckValue32(FIFO_USER_01) && frames++<120) swiWaitForVBlank();
            if(!fifoCheckValue32(FIFO_USER_01)) return;
            ids[j]=fifoGetValue32(FIFO_USER_01);
        }
        if((ids[0]|ids[1])!=0 && (ids[0]&ids[1])!=0xffffffff) { memcpy(studio_console_id,ids,sizeof(ids)); dsi_crypt_init(ids); studio_crypto=true; }
    }
    if(argc>0 && argv && argv[0] && !strncmp(argv[0],"fat:",4)) strcpy(studio_root,"fat:/");
}
static void help(void) {
    release_keys(); UiTouch touch={0}; int page=0;
    for(;;) {
        scanKeys(); UiAction hit=UI_NONE;
        if(keysHeld()&KEY_TOUCH) { touchPosition t; touchRead(&t); hit=ui_help_hit(t.px,t.py); }
        UiAction action=ui_touch_update(&touch,hit,keysDown()&KEY_TOUCH,keysHeld()&KEY_TOUCH,keysUp()&KEY_TOUCH);
        ui_set_pressed(touch.captured==touch.hover?touch.hover:UI_NONE);
        int previous=page;
        if(action>=UI_GUIDE0 && action<=UI_GUIDE4) page=action-UI_GUIDE0;
        if(keysDown()&(KEY_RIGHT|KEY_R)) page=(page+1)%5;
        if(keysDown()&(KEY_LEFT|KEY_L)) page=(page+4)%5;
        if(page!=previous) studio_sound(AUDIO_SELECT);
        if((keysDown()&(KEY_A|KEY_B)) || action==UI_ACCEPT) break;
        ui_help(page); studio_present();
    }
    ui_set_pressed(UI_NONE); release_keys();
}
static void title_screen(void) {
    release_keys(); UiTouch touch={0};
    for(;;) {
        studio_music(true); scanKeys(); UiAction hit=UI_NONE;
        if(keysHeld()&KEY_TOUCH) { touchPosition t; touchRead(&t); hit=ui_title_hit(t.px,t.py); }
        UiAction action=ui_touch_update(&touch,hit,keysDown()&KEY_TOUCH,keysHeld()&KEY_TOUCH,keysUp()&KEY_TOUCH);
        ui_set_pressed(touch.captured==touch.hover?touch.hover:UI_NONE);
        if((keysDown()&KEY_A) || action==UI_OPEN) {
            studio_sound(AUDIO_ACCEPT);
            if(!studio_nand || !studio_crypto) {
                studio_notice("BROWSING ONLY","This launch cannot install games or back up your DSi. Use a launch method with DSi storage access to set up installation.");
                break;
            }
            if(studio_backup(true) || studio_backup_ready()) break;
            touch=(UiTouch){0}; release_keys();
        }
        if((keysDown()&KEY_START) || action==UI_HELP) { help(); touch=(UiTouch){0}; }
        ui_title(); studio_present();
    }
    ui_set_pressed(UI_NONE); release_keys();
}
int main(int argc,char **argv) {
    defaultExceptionHandler();
    videoSetMode(MODE_5_2D); videoSetModeSub(MODE_5_2D);
    vramSetBankA(VRAM_A_MAIN_BG); vramSetBankC(VRAM_C_SUB_BG);
    top_bg=bgInit(2,BgType_Bmp16,BgSize_B16_256x256,0,0);
    bottom_bg=bgInitSub(2,BgType_Bmp16,BgSize_B16_256x256,0,0);
    lcdMainOnTop(); keysSetRepeat(22,6);
    ui_title(); ui_panel("DSi Studio","Opening your collection...",UI_PROGRESS,-1); studio_present();
    setup_storage(argc,argv); studio_audio_init(); title_screen(); studio_scan();
    UiTouch touch={0};
    while(1) {
        studio_music(true); scanKeys(); int k=keysDownRepeat(),previous=studio_selected;
        if(k&KEY_RIGHT && studio_count && studio_selected+1<studio_count) studio_selected++;
        if(k&KEY_LEFT && studio_selected>0) studio_selected--;
        if(k&KEY_DOWN && studio_selected+2<studio_count) studio_selected+=2;
        if(k&KEY_UP && studio_selected>=2) studio_selected-=2;
        UiAction hit=UI_NONE;
        if(keysHeld()&KEY_TOUCH) { touchPosition t; touchRead(&t); hit=ui_library_hit(t.px,t.py,studio_games,studio_count,studio_selected,studio_nand&&studio_crypto); }
        UiAction action=ui_touch_update(&touch,hit,keysDown()&KEY_TOUCH,keysHeld()&KEY_TOUCH,keysUp()&KEY_TOUCH);
        ui_set_pressed(touch.captured==touch.hover?touch.hover:UI_NONE);
        if(action>=UI_TILE0 && action<=UI_TILE3) studio_selected=studio_selected/4*4+action-UI_TILE0;
        if(action==UI_PREV) studio_selected=(studio_selected/4-1)*4;
        if(action==UI_NEXT) studio_selected=(studio_selected/4+1)*4;
        if(previous!=studio_selected) studio_sound(AUDIO_SELECT);
        if(keysDown()&(KEY_L|KEY_R)) action=UI_CARD;
        if(keysDown()&KEY_X) action=UI_REFRESH;
        if(keysDown()&KEY_Y) action=UI_BACKUP;
        if(keysDown()&KEY_START) action=UI_HELP;
        if(keysDown()&KEY_B) action=UI_HOME;
        if((keysDown()&KEY_A) && ui_can_copy(studio_games,studio_count,studio_selected,studio_nand&&studio_crypto)) action=UI_COPY;
        if(action==UI_CARD) {
            studio_backup_reset();
            const char *other=!strcmp(studio_root,"fat:/")?"sd:/":"fat:/";
            DIR *d=opendir(other);
            if(d) { closedir(d); strcpy(studio_root,other); studio_scan(); }
            else studio_notice("CARD NOT AVAILABLE","This launch method cannot access the other card. Put games in /DSiWare on the accessible card.");
        }
        if(action==UI_REFRESH) { studio_sound(AUDIO_ACCEPT); studio_scan(); }
        if(action==UI_BACKUP) { studio_sound(AUDIO_ACCEPT); studio_backup(false); }
        if(action==UI_HELP) help();
        if(action==UI_HOME) { studio_sound(AUDIO_BACK); title_screen(); studio_storage_read(&storage); }
        if(action==UI_COPY && studio_count) {
            studio_sound(AUDIO_ACCEPT); studio_install(&studio_games[studio_selected]); studio_storage_read(&storage);
        }
        if(studio_nand && studio_crypto && !studio_backup_ready()) title_screen();
        if(action>=UI_COPY && action!=UI_PREV && action!=UI_NEXT) { touch=(UiTouch){0}; ui_set_pressed(UI_NONE); release_keys(); }
        library(); studio_present();
    }
}
