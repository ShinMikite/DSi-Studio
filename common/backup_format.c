// SPDX-License-Identifier: GPL-3.0-or-later
#include "backup_format.h"
#include <string.h>
bool backup_nand_size(uint64_t size) { return size==0xf000000 || size==0xf580000; }
void backup_footer(uint8_t out[64], const uint8_t cid[16], const uint8_t console[8]) {
    memset(out,0,64); memcpy(out,"DSi eMMC CID/CPU",16);
    memcpy(out+16,cid,16); memcpy(out+32,console,8);
}
void backup_record(uint8_t out[112], uint64_t size, const uint8_t footer[64], const uint8_t hash[32]) {
    memcpy(out,"DSISTBK1",8);
    for(int i=0;i<8;i++) out[8+i]=(uint8_t)(size>>(i*8));
    memcpy(out+16,footer,64); memcpy(out+80,hash,32);
}
bool backup_record_matches(const uint8_t *record, size_t length, uint64_t size, const uint8_t footer[64]) {
    if(length!=112 || !backup_nand_size(size) || memcmp(record,"DSISTBK1",8) || memcmp(record+16,footer,64)) return false;
    uint64_t saved=0; for(int i=0;i<8;i++) saved|=(uint64_t)record[8+i]<<(i*8);
    return saved==size;
}
