// SPDX-License-Identifier: GPL-3.0-or-later
#include "backup_session.h"
#include <string.h>
void backup_session_clear(BackupSession *s) { memset(s,0,sizeof(*s)); }
bool backup_session_matches(const BackupSession *s,const char *root,
                            const uint8_t record[112],BackupStamp stamp) {
    return s->valid && !strcmp(s->root,root) && !memcmp(s->record,record,112) &&
           s->stamp.size==stamp.size && s->stamp.modified==stamp.modified;
}
void backup_session_store(BackupSession *s,const char *root,
                          const uint8_t record[112],BackupStamp stamp) {
    backup_session_clear(s);
    if(strlen(root)>=sizeof(s->root)) return;
    strcpy(s->root,root); memcpy(s->record,record,112); s->stamp=stamp; s->valid=true;
}
