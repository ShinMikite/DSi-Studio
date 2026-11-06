// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <stdbool.h>
#include <stdint.h>

#define STUDIO_GAME_SPACE (128ULL*1024*1024)
#define STUDIO_NAND_RESERVE (1024ULL*1024)
#define STUDIO_MENU_SLOTS 39u
#define STUDIO_BLOCK_BYTES (128u*1024)

typedef struct {
    uint64_t free_bytes;
    uint32_t cluster_size;
    unsigned free_slots;
    bool known;
} StudioStorage;

bool studio_storage_capacity(uint64_t free_clusters,uint64_t cluster_size,
                             uint64_t used_game_bytes,unsigned titles,StudioStorage *out);
