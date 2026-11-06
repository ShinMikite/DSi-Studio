// SPDX-License-Identifier: GPL-3.0-or-later
#include "game_files.h"
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static bool equal_fold(const char *a,const char *b) {
    while(*a && *b) {
        if(tolower((unsigned char)*a++)!=tolower((unsigned char)*b++)) return false;
    }
    return *a==*b;
}
static bool metadata_name(const char *name) {
    const char *dot=strrchr(name,'.');
    if(equal_fold(name,"tmd") || (dot && equal_fold(dot,".tmd"))) return true;
    if(strlen(name)<5 || tolower((unsigned char)name[0])!='t' ||
       tolower((unsigned char)name[1])!='m' || tolower((unsigned char)name[2])!='d' || name[3]!='.') return false;
    for(const char *p=name+4;*p;p++) if(!isdigit((unsigned char)*p)) return false;
    return true;
}
static bool fail(StudioGame *g,const char *message) {
    g->metadata=false;
    snprintf(g->issue,sizeof(g->issue),"%s",message);
    return false;
}
bool studio_game_metadata(StudioGame *g,uint8_t record[520]) {
    // Scanning and installation run sequentially. Keep scratch storage off
    // the stack because the library can be walking several nested folders.
    static uint8_t data[2312];
    static StudioGame candidate;
    g->metadata=false;
    char folder[STUDIO_PATH];
    const char *slash=strrchr(g->path,'/');
    size_t length=slash?(size_t)(slash-g->path+1):0;
    if(length>=sizeof(folder)) return fail(g,"The game folder path is too long.");
    if(length) { memcpy(folder,g->path,length); folder[length]=0; }
    else strcpy(folder,"./");
    DIR *dir=opendir(folder);
    if(!dir) return fail(g,"Could not read this game folder. Reinsert the card and scan again.");
    uint32_t content_id=0;
    bool numbered=studio_content_name(g->path,&content_id),found=false;
    const char *error=NULL;
    for(;;) {
        errno=0; struct dirent *entry=readdir(dir);
        if(!entry) { if(errno) error="Could not finish reading this game folder. Scan again."; break; }
        if(entry->d_name[0]=='.' || !metadata_name(entry->d_name)) continue;
        char path[STUDIO_PATH];
        int n=snprintf(path,sizeof(path),"%s%s",folder,entry->d_name);
        if(n<0 || n>=(int)sizeof(path)) { error="The game folder path is too long."; break; }
        struct stat st;
        if(stat(path,&st)) { error="Could not read the game's information files. Copy the folder again."; break; }
        if(!S_ISREG(st.st_mode) || (st.st_size!=520 && st.st_size!=2312)) continue;
        FILE *file=fopen(path,"rb");
        if(!file) { error="Could not read the game's information files. Copy the folder again."; break; }
        bool ok=fread(data,1,(size_t)st.st_size,file)==(size_t)st.st_size;
        if(fclose(file)) ok=false;
        if(!ok) { error="Could not read the game's information files. Copy the folder again."; break; }
        candidate=*g;
        if(!studio_tmd(data,(size_t)st.st_size,&candidate)) continue;
        if(numbered && candidate.content_id!=content_id) continue;
        // Identical records with/without certificates are harmless duplicates.
        // Never pick an arbitrary version when multiple records could match.
        if(found && memcmp(record,data,520)) {
            error="More than one game version matches. Keep each version in its own folder."; break;
        }
        memcpy(record,data,520); found=true;
    }
    if(closedir(dir)) error="Could not finish reading this game folder. Scan again.";
    if(error) return fail(g,error);
    if(!found) return fail(g,"Matching game information is missing. Copy the complete extracted folder again.");
    return studio_tmd(record,520,g);
}
