#pragma once

namespace field_interaction
{
// The native HUD has no 7.69 quick-slot widgets. Its Q/W/E/R/T handlers
// must receive the key even when the corresponding modern widget is absent.
constexpr int QuickSlotIndex(bool nativeHUD, char key)
{
    if (nativeHUD) return -1;
    switch (key)
    {
    case 'Q': case 'q': return 0;
    case 'W': case 'w': return 1;
    case 'E': case 'e': return 2;
    case 'R': case 'r': return 3;
    case 'T': case 't': return 4;
    default: return -1;
    }
}

// 0x3AE also acknowledges recall, logout and server selection. An ack alone
// is not permission to close the application: a local quit must be pending.
constexpr bool ShouldCloseOnDelayAck(unsigned int quitStartedAt)
{
    return quitStartedAt != 0;
}
}
