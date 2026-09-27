#pragma once

#include <cstddef>

// S->C slot envelope: header(12), DestType(2), DestPos(2), Item(8).
// STRUCT_ITEM remains in Basedef; this metadata does not duplicate its layout.
// Native 004B263E consumes offsets 12/14/16; Go sends 24 bytes.
constexpr auto MSG_SendItem_Opcode = 0x182;
constexpr std::size_t kSendItemPacketSize = 24;

// Reject unsupported storage types as well as out-of-range signed positions.
// Callers supply their real array capacities, including reserved local slots.
constexpr bool IsSendItemDestination(int type, int position,
    std::size_t equipmentCount, std::size_t carryCount, std::size_t cargoCount)
{
    if (position < 0)
        return false;
    const auto slot = static_cast<std::size_t>(position);
    switch (type)
    {
    case 0: return slot < equipmentCount;
    case 1: return slot < carryCount;
    case 2: return slot < cargoCount;
    default: return false;
    }
}
