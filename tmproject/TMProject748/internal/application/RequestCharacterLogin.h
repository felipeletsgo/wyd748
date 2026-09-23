#pragma once

#include "ports/ICharacterLoginSender.h"

// Synchronous use case: validate the login intent and request one send.
// The scene still owns debouncing, character existence, buttons, and timing.
// This neither changes scenes nor modifies authoritative session or character state.
inline bool RequestCharacterLogin(ICharacterLoginSender& sender, int slot)
{
    // Reusable guard; the live scene already rejects these indices before calling.
    if (slot < 0 || slot >= 4)
        return false;

    // Propagate failure without retry. Encoding and buffer lifetime belong to
    // the wire adapter; the use case imports no protocol or platform details.
    return sender.SendCharacterLogin(slot);
}
