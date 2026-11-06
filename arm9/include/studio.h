// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "catalog.h"
#include "ui.h"
#include <nds.h>
extern StudioGame studio_games[STUDIO_MAX_GAMES];
extern int studio_count, studio_selected, studio_region;
extern bool studio_nand, studio_crypto;
extern uint32_t studio_console_id[2];
extern char studio_root[8];
void studio_present(void);
void studio_notice(const char *title,const char *body);
bool studio_confirm(const char *title,const char *body);
bool studio_choice(const char *title,const char *body);
void studio_progress(const char *text,int percent);
void studio_scan(void);
bool studio_install(StudioGame *g);
bool studio_storage_read(StudioStorage *storage);
bool studio_backup(bool required);
bool studio_backup_ready(void);
bool studio_backup_check_cached(void);
void studio_backup_reset(void);
bool studio_read(const char *path,void *out,size_t size);
uint64_t studio_size(const char *path);
