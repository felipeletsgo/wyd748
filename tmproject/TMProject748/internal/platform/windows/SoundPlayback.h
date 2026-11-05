#pragma once
#include <windows.h>

namespace sound_playback {
// DSBSTATUS_PLAYING from dsound.h; looping buffers also report this bit.
constexpr DWORD PlayingStatus = 0x00000001;

// True when any present buffer reports the playing bit. A missing buffer
// table or a failed status query never reports playback.
template<class Buffer>
bool AnyPlaying(Buffer* const* buffers, DWORD count)
{
    if (!buffers) return false;
    for (DWORD i = 0; i < count; ++i) {
        if (!buffers[i]) continue;
        DWORD status = 0;
        if (FAILED(buffers[i]->GetStatus(&status))) continue;
        if (status & PlayingStatus) return true;
    }
    return false;
}
}
