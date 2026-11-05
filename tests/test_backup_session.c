#include "backup_session.h"
#include "backup_format.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void) {
    BackupSession s={0};
    uint8_t footer[64],record[112],other[112],cid[16]={1},console[8]={2},hash[32]={3};
    BackupStamp original={0xf000040,123456},changed=original;
    backup_footer(footer,cid,console); backup_record(record,0xf000000,footer,hash);
    assert(!backup_session_matches(&s,"fat:/",record,original));
    backup_session_store(&s,"fat:/",record,original);
    for(int install=0;install<5;install++) assert(backup_session_matches(&s,"fat:/",record,original));
    // Multiple consoles sharing one game card must never share verification.
    console[0]++; backup_footer(footer,cid,console); backup_record(other,0xf000000,footer,hash);
    assert(!backup_session_matches(&s,"fat:/",other,original));
    assert(!backup_session_matches(&s,"sd:/",record,original));
    changed.size--; assert(!backup_session_matches(&s,"fat:/",record,changed));
    changed=original; changed.modified++; assert(!backup_session_matches(&s,"fat:/",record,changed));
    memcpy(other,record,112); other[80]^=1; assert(!backup_session_matches(&s,"fat:/",other,original));
    backup_session_clear(&s); assert(!backup_session_matches(&s,"fat:/",record,original));
    // Returning to the original card after invalidation requires a new check.
    backup_session_store(&s,"sd:/",record,original); backup_session_clear(&s);
    assert(!backup_session_matches(&s,"fat:/",record,original));
    backup_session_store(&s,"too-long-root",record,original); assert(!s.valid);
    puts("Backup session: repeated installs reuse verification; console/card changes, changed records/files and invalidation require a new check.");
}
