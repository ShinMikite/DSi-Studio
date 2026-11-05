// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <stdbool.h>
#include <stdint.h>
typedef struct { uint64_t size; int64_t modified; } BackupStamp;
typedef struct {
    bool valid;
    char root[8];
    uint8_t record[112];
    BackupStamp stamp;
} BackupSession;
void backup_session_clear(BackupSession *session);
bool backup_session_matches(const BackupSession *session,const char *root,
                            const uint8_t record[112],BackupStamp stamp);
void backup_session_store(BackupSession *session,const char *root,
                          const uint8_t record[112],BackupStamp stamp);
