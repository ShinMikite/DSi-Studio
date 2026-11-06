#include "ui.h"
#include "messages.h"
#include <stdio.h>
#include <string.h>
static void save(const char *path) {
    FILE *f=fopen(path,"wb"); fprintf(f,"P6\n256 384\n255\n");
    for(int y=0;y<384;y++) for(int x=0;x<256;x++) {
        uint16_t v=(y<192?studio_top:studio_bottom)[(y%192)*256+x];
        unsigned char rgb[]={(v&31)*255/31,((v>>5)&31)*255/31,((v>>10)&31)*255/31}; fwrite(rgb,1,3,f);
    } fclose(f);
}
int main(void) {
    StudioGame g[5]={0}; const char *names[]={"Starfall Express","Pocket Garden","Cloud Painter","Tiny Music Box","A very long game title for wrapping"};
    for(int i=0;i<5;i++) { strcpy(g[i].title,names[i]); g[i].size=(i+1)*1048576; g[i].metadata=i!=3;
        for(int y=0;y<32;y++) for(int x=0;x<32;x++) g[i].icon[y*32+x]=0x8000|((5+i*4)&31)|(((x+y)/3&31)<<5)|((20+i*2)<<10);
    }
    g[1].installed=true;
    StudioStorage storage={.free_bytes=83*1048576,.cluster_size=4096,.free_slots=20,.known=true};
    ui_library(g,5,0,"FLASHCARD SD",true,&storage); save("artifacts/library.ppm");
    ui_panel("COPY TO DSi MENU?",STUDIO_COPY_WARNING,UI_HOLD,40); save("artifacts/confirm.ppm");
    ui_library(g,0,0,"FLASHCARD SD",false,NULL); save("artifacts/empty.ppm");
    ui_library(g,5,4,"DSi SD CARD",false,NULL); save("artifacts/last-page.ppm");
    storage.free_bytes=128*1048576;
    ui_library(g,5,0,"FLASHCARD SD",true,&storage); save("artifacts/storage-full-capacity.ppm");
    storage.free_bytes=4096;
    ui_library(g,5,0,"FLASHCARD SD",true,&storage); save("artifacts/storage-low.ppm");
    storage.free_slots=0;
    ui_library(g,5,0,"FLASHCARD SD",true,&storage); save("artifacts/storage-no-slots.ppm");
    strcpy(g[0].issue,"More than one game version matches. Keep each version in its own folder.");
    ui_library(g,5,0,"FLASHCARD SD",true,&storage); save("artifacts/storage-with-issue.ppm");
    ui_title(); save("artifacts/title.ppm");
    ui_help(0); save("artifacts/help-install.ppm");
    ui_help(1); save("artifacts/help.ppm");
    ui_help(2); save("artifacts/help-backup.ppm");
    ui_help(3); save("artifacts/help-safety.ppm");
    ui_help(4); save("artifacts/help-recovery.ppm");
    ui_panel("PREPARING BACKUP","Checking this DSi and your card...\n\nYour console has not been changed.",UI_PROGRESS,-1); save("artifacts/backup-preparing.ppm");
    ui_panel("BACK UP THIS DSi","Saving this DSi to your card...",UI_PROGRESS_CANCEL,62); save("artifacts/backup-progress.ppm");
    ui_panel("FIRST: BACK UP THIS DSi",STUDIO_BACKUP_INTRO,UI_CHOICE,-1); save("artifacts/backup-choice.ppm");
    return 0;
}
