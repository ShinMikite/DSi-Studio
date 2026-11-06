// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "catalog.h"

// Resolve metadata beside g->path. Returns its signed 520-byte record.
// This only identifies a candidate; the installer must verify its signature
// and the complete game's hash before writing to internal storage.
bool studio_game_metadata(StudioGame *g, uint8_t record[520]);
