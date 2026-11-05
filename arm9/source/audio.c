// SPDX-License-Identifier: GPL-3.0-or-later
#include <nds.h>
#include "audio.h"
#include "studio_music_bin.h"
#include "studio_select_bin.h"
#include "studio_accept_bin.h"
#include "studio_back_bin.h"
#include "studio_success_bin.h"
#include "studio_error_bin.h"
static bool initialized=false,playing=false;
void studio_audio_init(void) {
    soundEnable(); initialized=true;
}
void studio_music(bool on) {
    if(!initialized || on==playing) return;
    if(on) soundPlaySampleChannel(0,studio_music_bin,SoundFormat_16Bit,studio_music_bin_size,16384,85,64,true,0);
    else soundKill(0);
    playing=on;
}
void studio_sound(StudioSound s) {
    if(!initialized) return;
    const void *data; unsigned size;
    switch(s) {
        case AUDIO_SELECT:data=studio_select_bin;size=studio_select_bin_size;break;
        case AUDIO_ACCEPT:data=studio_accept_bin;size=studio_accept_bin_size;break;
        case AUDIO_BACK:data=studio_back_bin;size=studio_back_bin_size;break;
        case AUDIO_SUCCESS:data=studio_success_bin;size=studio_success_bin_size;break;
        default:data=studio_error_bin;size=studio_error_bin_size;break;
    }
    soundPlaySampleChannel(1,data,SoundFormat_16Bit,size,16384,110,64,false,0);
}
