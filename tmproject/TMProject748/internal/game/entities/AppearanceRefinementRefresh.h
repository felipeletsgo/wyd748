#pragma once

#include "../../core/Structures.h"

namespace appearance_refinement {

// Native 0x00480a83 preserves the active block, not the packet-state cache.
// Rebuild first so the cache retains fresh equipment for shadow restoration.
template <typename Human, typename Mob>
void Rebuild(Human& human, Mob& mob)
{
    const SANC_INFO preserved = human.m_stSancInfo;
    human.SetPacketMOBItem(&mob);
    if (preserved.Sanc0 != 0 && mob.Equip[0].sIndex != 32)
        human.m_stSancInfo = preserved;
}

} // namespace appearance_refinement
