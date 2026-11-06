// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#define STUDIO_MAX_GAMES 256
#define STUDIO_PATH 512
typedef struct {
    char path[STUDIO_PATH], title[129];
    uint32_t id, regions, public_size, private_size, content_id;
    uint64_t size;
    uint8_t appflags, hash[20];
    uint16_t icon[1024];
    bool metadata, installed;
    char issue[96];
} StudioGame;
uint32_t studio_le32(const uint8_t *p);
uint32_t studio_be32(const uint8_t *p);
uint64_t studio_be64(const uint8_t *p);
bool studio_header(const uint8_t *p, size_t n, uint64_t size, StudioGame *g);
bool studio_tmd(const uint8_t *p, size_t n, StudioGame *g);
bool studio_sidecar(const char *path, const char *ext, char *out, size_t cap);
bool studio_extension(const char *path);
bool studio_content_name(const char *path, uint32_t *id);
bool studio_game_file(const char *path);
