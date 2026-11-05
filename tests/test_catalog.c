#include "catalog.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void le(uint8_t *p,uint32_t v) { for(int i=0;i<4;i++) p[i]=v>>(i*8); }
static void be(uint8_t *p,uint32_t v) { for(int i=0;i<4;i++) p[3-i]=v>>(i*8); }
int main(void) {
    uint8_t h[0x240]={0},t[2312]={0}; StudioGame g={0};
    h[0x12]=3; le(h+0x230,0x4b414245); le(h+0x234,0x30004); le(h+0x1b0,2);
    assert(studio_header(h,sizeof(h),0x4000,&g));
    assert(!studio_header(h,32,0x4000,&g));
    le(h+0x234,0x30017); assert(!studio_header(h,sizeof(h),0x4000,&g)); le(h+0x234,0x30004);
    le(h+0x238,0xffffffff); assert(!studio_header(h,sizeof(h),0x4000,&g)); le(h+0x238,0);
    assert(!studio_header(h,sizeof(h),0x200,&g)); assert(studio_header(h,sizeof(h),0x4000,&g));
    be(t,0x10001); memcpy(t+0x140,"Root-CA00000001-CP00000007",26);
    be(t+0x18c,0x30004); be(t+0x190,g.id); t[0x1df]=1; t[0x1eb]=1; be(t+0x1f0,0x4000); be(t+0x1e4,0x12345678);
    assert(studio_tmd(t,520,&g) && g.content_id==0x12345678);
    assert(studio_tmd(t,2312,&g));
    assert(!studio_tmd(t,519,&g));
    t[0x1df]=2; assert(!studio_tmd(t,520,&g)); t[0x1df]=1;
    t[0x190]^=1; assert(!studio_tmd(t,520,&g)); t[0x190]^=1;
    t[0x1ec]=1; assert(!studio_tmd(t,520,&g)); t[0x1ec]=0;
    t[0x1e9]=1; assert(!studio_tmd(t,520,&g)); t[0x1e9]=0;
    t[0x140]='X'; assert(!studio_tmd(t,520,&g));
    char out[64]; assert(studio_sidecar("fat:/DSiWare/test.NDS",".tmd",out,sizeof(out)));
    assert(!strcmp(out,"fat:/DSiWare/test.tmd"));
    assert(!studio_sidecar("fat:/dotted.dir/test",".tmd",out,sizeof(out)));
    assert(!studio_sidecar("fat:/DSiWare/test.nds",".tmd",out,5));
    assert(studio_extension("game.NDS") && studio_extension("game.app"));
    assert(!studio_extension("noextension") && !studio_extension("test.zip") && !studio_extension("test.ndsx"));
    puts("Catalog validation: all 22 assertions passed.");
    return 0;
}
