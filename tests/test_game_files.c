// SPDX-License-Identifier: GPL-3.0-or-later
#include "game_files.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

int main(int argc,char **argv) {
    if(argc!=2 || strlen(argv[1])>=STUDIO_PATH || !studio_game_file(argv[1])) return 2;
    StudioGame game={0}; uint8_t header[0x240],record[520]; struct stat st;
    if(stat(argv[1],&st) || !S_ISREG(st.st_mode)) return 2;
    FILE *f=fopen(argv[1],"rb"); if(!f) return 2;
    bool ok=fread(header,1,sizeof(header),f)==sizeof(header);
    if(fclose(f)) ok=false;
    if(!ok || !studio_header(header,sizeof(header),st.st_size,&game)) return 2;
    snprintf(game.path,sizeof(game.path),"%s",argv[1]);
    if(!studio_game_metadata(&game,record)) { puts(game.issue); return 1; }
    printf("%08lx %08lx ",(unsigned long)game.id,(unsigned long)game.content_id);
    for(int i=0;i<20;i++) printf("%02x",game.hash[i]);
    puts(""); return 0;
}
