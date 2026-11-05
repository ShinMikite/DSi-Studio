#include "backup_format.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void) {
    uint8_t cid[16],console[8],footer[64],record[112],hash[32];
    for(int i=0;i<16;i++) cid[i]=(uint8_t)(i+1);
    for(int i=0;i<8;i++) console[i]=(uint8_t)(i+21);
    memset(hash,0x75,sizeof(hash));
    backup_footer(footer,cid,console);
    assert(!memcmp(footer,"DSi eMMC CID/CPU",16));
    assert(!memcmp(footer+16,cid,16)); assert(!memcmp(footer+32,console,8));
    for(int i=40;i<64;i++) assert(footer[i]==0);
    assert(backup_nand_size(0xf000000)); assert(backup_nand_size(0xf580000));
    assert(!backup_nand_size(0)); assert(!backup_nand_size(0x10000000));
    backup_record(record,0xf000000,footer,hash);
    assert(backup_record_matches(record,112,0xf000000,footer));
    assert(!memcmp(record+80,hash,32));
    assert(!backup_record_matches(record,111,0xf000000,footer));
    assert(!backup_record_matches(record,112,0xf580000,footer));
    footer[16]^=1; assert(!backup_record_matches(record,112,0xf000000,footer)); footer[16]^=1;
    footer[32]^=1; assert(!backup_record_matches(record,112,0xf000000,footer)); footer[32]^=1;
    record[0]^=1; assert(!backup_record_matches(record,112,0xf000000,footer));
    puts("Backup format: identity, sizes, footer, truncated record and version checks passed.");
}
