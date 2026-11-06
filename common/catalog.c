// SPDX-License-Identifier: GPL-3.0-or-later
#include "catalog.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
uint32_t studio_le32(const uint8_t *p) { return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24; }
uint32_t studio_be32(const uint8_t *p) { return (uint32_t)p[3]|(uint32_t)p[2]<<8|(uint32_t)p[1]<<16|(uint32_t)p[0]<<24; }
uint64_t studio_be64(const uint8_t *p) { return (uint64_t)studio_be32(p)<<32|studio_be32(p+4); }
static bool fail(StudioGame *g,const char *s) { snprintf(g->issue,sizeof(g->issue),"%s",s); return false; }
bool studio_header(const uint8_t *p,size_t n,uint64_t size,StudioGame *g) {
    if(n<0x240 || size<0x1000 || size>128*1024*1024) return fail(g,"Incomplete or oversized game.");
    if(p[0x12]!=2 && p[0x12]!=3) return fail(g,"This is not a DSi title.");
    if(studio_le32(p+0x234)!=0x00030004) return fail(g,"Only DSiWare games are supported.");
    g->id=studio_le32(p+0x230); g->regions=studio_le32(p+0x1b0);
    g->public_size=studio_le32(p+0x238); g->private_size=studio_le32(p+0x23c);
    g->appflags=p[0x1bf]; g->size=size;
    if(!g->id || g->id==0x23232323) return fail(g,"Missing retail title ID.");
    // Save formatting uses FAT12 and 16-bit sector geometry (NTM/GodMode9).
    uint32_t saves[]={g->public_size,g->private_size};
    for(int i=0;i<2;i++) if(saves[i] && (saves[i]<4096 || saves[i]>16*1024*1024 || saves[i]%512)) return fail(g,"Unsupported save size.");
    for(int i=0;i<12;i++) g->title[i]=(p[i]>=32 && p[i]<127)?p[i]:' ';
    g->title[12]=0; g->issue[0]=0; return true;
}
bool studio_tmd(const uint8_t *p,size_t n,StudioGame *g) {
    g->metadata=false;
    if(n!=520 && n!=2312) return fail(g,"Missing or invalid .tmd file.");
    if(studio_be32(p)!=0x10001 || p[0x180]!=0 || memcmp(p+0x140,"Root-CA00000001-CP00000007\0",26)) return fail(g,"Unsupported retail metadata.");
    if(studio_be32(p+0x18c)!=0x30004 || studio_be32(p+0x190)!=g->id) return fail(g,"Metadata belongs to another game.");
    if(p[0x1de]!=0 || p[0x1df]!=1 || p[0x1e0]!=0 || p[0x1e1]!=0 || p[0x1e8]!=0 || p[0x1e9]!=0 || p[0x1ea]!=0 || p[0x1eb]!=1) return fail(g,"Unsupported content layout.");
    if(studio_be64(p+0x1ec)!=g->size) return fail(g,"Game size differs from metadata.");
    g->content_id=studio_be32(p+0x1e4); memcpy(g->hash,p+0x1f4,20);
    g->metadata=true; g->issue[0]=0; return true;
}
bool studio_sidecar(const char *path,const char *ext,char *out,size_t cap) {
    const char *dot=strrchr(path,'.'), *slash=strrchr(path,'/');
    if(!dot || (slash && dot<slash)) dot=path+strlen(path);
    size_t n=dot-path, e=strlen(ext); if(n+e+1>cap) return false;
    memcpy(out,path,n); memcpy(out+n,ext,e+1); return true;
}
bool studio_extension(const char *path) {
    const char *d=strrchr(path,'.'); if(!d || strlen(d)!=4) return false;
    char e[5]; for(int i=0;i<5;i++) e[i]=tolower((unsigned char)d[i]);
    return !strcmp(e,".nds") || !strcmp(e,".dsi") || !strcmp(e,".app") || !strcmp(e,".srl");
}
bool studio_content_name(const char *path,uint32_t *id) {
    const char *name=strrchr(path,'/'); name=name?name+1:path;
    size_t n=strlen(name);
    if(n!=8 && !(n==12 && name[8]=='.' &&
       tolower((unsigned char)name[9])=='a' && tolower((unsigned char)name[10])=='p' &&
       tolower((unsigned char)name[11])=='p')) return false;
    uint32_t value=0;
    for(int i=0;i<8;i++) {
        unsigned char c=(unsigned char)tolower((unsigned char)name[i]);
        if(!((c>='0' && c<='9') || (c>='a' && c<='f'))) return false;
        value=(value<<4)|(c<='9'?c-'0':c-'a'+10);
    }
    if(id) *id=value;
    return true;
}
bool studio_game_file(const char *path) {
    return studio_extension(path) || studio_content_name(path,NULL);
}
