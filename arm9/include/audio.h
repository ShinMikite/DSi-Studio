// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <stdbool.h>
typedef enum { AUDIO_SELECT,AUDIO_ACCEPT,AUDIO_BACK,AUDIO_SUCCESS,AUDIO_ERROR } StudioSound;
void studio_audio_init(void);
void studio_music(bool playing);
void studio_sound(StudioSound sound);
