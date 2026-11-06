// SPDX-License-Identifier: GPL-3.0-or-later
#include "storage.h"

bool studio_storage_capacity(uint64_t free_clusters,uint64_t cluster_size,
                             uint64_t used_game_bytes,unsigned titles,StudioStorage *out) {
    *out=(StudioStorage){0};
    if(!cluster_size || cluster_size>UINT32_MAX || free_clusters>UINT64_MAX/cluster_size) return false;
    uint64_t free_bytes=free_clusters*cluster_size;
    free_bytes=free_bytes>STUDIO_NAND_RESERVE?free_bytes-STUDIO_NAND_RESERVE:0;
    uint64_t game_space=used_game_bytes<STUDIO_GAME_SPACE?STUDIO_GAME_SPACE-used_game_bytes:0;
    out->free_bytes=free_bytes<game_space?free_bytes:game_space;
    out->cluster_size=(uint32_t)cluster_size;
    out->free_slots=titles<STUDIO_MENU_SLOTS?STUDIO_MENU_SLOTS-titles:0;
    out->known=true;
    return true;
}
