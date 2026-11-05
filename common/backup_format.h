// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define STUDIO_FOOTER_SIZE 64
#define STUDIO_BACKUP_RECORD_SIZE 112
bool backup_nand_size(uint64_t size);
void backup_footer(uint8_t out[64], const uint8_t cid[16], const uint8_t console[8]);
void backup_record(uint8_t out[112], uint64_t size, const uint8_t footer[64], const uint8_t hash[32]);
bool backup_record_matches(const uint8_t *record, size_t length, uint64_t size, const uint8_t footer[64]);
