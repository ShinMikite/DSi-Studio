// SPDX-License-Identifier: GPL-3.0-or-later
// Executes the production installer against a disposable host filesystem.
// Crypto, save formatting, battery, FIFO and NAND protection are substitutes;
// these tests exercise control flow and I/O failures, not hardware behavior.
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include "installer_platform.h"

static int fault_at, operations;
static bool commit_failure, remove_failure, low_space;
static bool fail_io(void) { operations++; if(operations==fault_at) { errno=EIO; return true; } return false; }
static const char *mapped(const char *path,char *out) {
    if(!strncmp(path,"nand:/",6)) { snprintf(out,512,"nand/%s",path+6); return out; }
    return path;
}
static void writable(void) { assert(!protected && guarded); }
static FILE *host_open(const char *path,const char *mode) {
    char p[512];
    if(strchr(mode,'w') || strchr(mode,'+')) writable();
    if(fail_io()) return NULL;
    return fopen(mapped(path,p),mode);
}
static size_t host_read(void *p,size_t s,size_t n,FILE *f) { return fail_io()?0:fread(p,s,n,f); }
static size_t host_write(const void *p,size_t s,size_t n,FILE *f) { writable(); return fail_io()?0:fwrite(p,s,n,f); }
static int host_flush(FILE *f) { writable(); return fail_io()?EOF:fflush(f); }
static int host_close(FILE *f) { bool failed=fail_io(); int result=fclose(f); return failed?EOF:result; }
static int host_access(const char *path,int mode) { char p[512]; return access(mapped(path,p),mode); }
static int host_stat(const char *path,struct stat *st) { char p[512]; return stat(mapped(path,p),st); }
static DIR *host_opendir(const char *path) { char p[512]; return fail_io()?NULL:opendir(mapped(path,p)); }
static int host_mkdir(const char *path,mode_t mode) { char p[512]; writable(); return fail_io()?-1:mkdir(mapped(path,p),mode); }
static int host_rename(const char *src,const char *dst) {
    char a[512],b[512]; writable();
    if(fail_io() || (commit_failure && strstr(dst,"nand:/title/"))) return -1;
    return rename(mapped(src,a),mapped(dst,b));
}
static int host_remove(const char *path) { char p[512]; writable(); return fail_io() || remove_failure?-1:remove(mapped(path,p)); }
static int host_statvfs(const char *path,struct statvfs *fs) {
    (void)path; memset(fs,0,sizeof(*fs)); fs->f_bsize=4096; fs->f_bavail=low_space?0:65536;
    return fail_io()?-1:0;
}
static uint64_t studio_size(const char *path) { struct stat st; return host_stat(path,&st)?0:(uint64_t)st.st_size; }
static bool studio_read(const char *path,void *out,size_t size) {
    FILE *f=host_open(path,"rb"); if(!f) return false;
    bool ok=host_read(out,1,size,f)==size; if(host_close(f)) ok=false; return ok;
}

#define fopen host_open
#define fread host_read
#define fwrite host_write
#define fflush host_flush
#define fclose host_close
#define access host_access
#define stat(path,st) host_stat(path,st)
#define opendir host_opendir
#define mkdir host_mkdir
#define rename host_rename
#define remove host_remove
#define statvfs(path,fs) host_statvfs(path,fs)
#define STUDIO_INSTALLER_HOST_TEST
#include "../arm9/source/installer.c"

int main(int argc,char **argv) {
    assert(argc==3);
    fault_at=atoi(argv[1]);
    const char *scenario=argv[2];
    backup_ok=strcmp(scenario,"backup")!=0;
    cached_ok=strcmp(scenario,"cached-backup")!=0;
    confirm_ok=strcmp(scenario,"cancel")!=0;
    charger=strcmp(scenario,"charger")!=0;
    signature_ok=strcmp(scenario,"signature")!=0;
    digest_ok=strcmp(scenario,"digest")!=0;
    guard_ack=strcmp(scenario,"guard")!=0;
    low_space=!strcmp(scenario,"space");
    commit_failure=!strcmp(scenario,"commit") || !strcmp(scenario,"rollback");
    remove_failure=!strcmp(scenario,"rollback");
    StudioGame game={0}; strcpy(game.path,"card/game.nds");
    bool result=studio_install(&game);
    assert(protected && !guarded);
    assert(game.installed==result || !strcmp(notice,"ALREADY ON YOUR DSi"));
    printf("%d %d %u %u %s\n",result,operations,unlocks,resets,notice);
    return 0;
}
