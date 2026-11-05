#pragma once

// Packet-side state decisions of TMHuman without scene dependencies.
namespace illusion_mana
{
    // 0x368 reaches every observer of the caster. Only the local caster spends
    // its own mana locally; the server resends authoritative vitals afterwards.
    inline unsigned int Apply(unsigned int currentMp, int cost, bool localCaster)
    {
        if (!localCaster || cost <= 0)
            return currentMp;
        const unsigned int spent = static_cast<unsigned int>(cost);
        return currentMp > spent ? currentMp - spent : 0;
    }
}

namespace selection_slot
{
    // ObjectManager uses -1 as the "no character" sentinel; the account
    // selection cache has four slots.
    constexpr int kSlots = 4;

    inline bool IsValid(int slot)
    {
        return slot >= 0 && slot < kSlots;
    }
}
