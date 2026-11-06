// SPDX-License-Identifier: GPL-3.0-or-later
#include "storage.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    StudioStorage s;
    const uint64_t mb=1024*1024;
    assert(studio_storage_capacity(256*mb/4096,4096,0,10,&s));
    assert(s.known && s.free_bytes==128*mb && s.free_slots==29 && s.cluster_size==4096);
    assert(studio_storage_capacity(64*mb/4096,4096,0,10,&s) && s.free_bytes==63*mb);
    assert(studio_storage_capacity(256*mb/4096,4096,100*mb,10,&s) && s.free_bytes==28*mb);
    assert(studio_storage_capacity(256*mb/4096,4096,128*mb,39,&s) && s.free_bytes==0 && s.free_slots==0);
    assert(studio_storage_capacity(256*mb/4096,4096,129*mb,40,&s) && s.free_bytes==0 && s.free_slots==0);
    assert(studio_storage_capacity(mb/4096,4096,0,0,&s) && s.free_bytes==0);
    assert(studio_storage_capacity(mb/4096-1,4096,0,0,&s) && s.free_bytes==0);
    assert(studio_storage_capacity(0,4096,0,0,&s) && s.free_bytes==0);
    assert(!studio_storage_capacity(100,0,0,0,&s) && !s.known);
    assert(!studio_storage_capacity(UINT64_MAX,4096,0,0,&s) && !s.known);
    assert(!studio_storage_capacity(1,UINT64_MAX,0,0,&s) && !s.known);
    // Capacity uses precisely the same two space limits as installation.
    for(uint64_t available=0;available<256*mb;available+=7*mb) {
        for(uint64_t used=0;used<150*mb;used+=9*mb) {
            assert(studio_storage_capacity(available/4096,4096,used,12,&s));
            for(uint64_t needed=4096;needed<128*mb;needed+=3*mb) {
                bool original=needed+STUDIO_NAND_RESERVE<=available && used+needed<=STUDIO_GAME_SPACE;
                assert((needed<=s.free_bytes)==original);
            }
        }
    }
    puts("Storage: filesystem reserve, game budget, menu slots and capacity boundaries passed.");
    return 0;
}
