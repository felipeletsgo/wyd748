#pragma once

#include "AttackFrameContract.h"
#include <cstdint>
#include <cstring>

namespace attack_visual
{
// Target slots that the opcode's frame carries. Unknown opcodes keep the
// historical 13-slot bound because this helper only projects visuals.
constexpr int TargetSlotLimit(unsigned int opcode)
{
    return IsAttackOpcode(opcode) ? static_cast<int>(AttackTargetCapacity(opcode))
                                  : static_cast<int>(kAttackMultiTargetCapacity);
}

// Reads DMGX slot `index` behind the opcode's native prefix. Returns false for
// an unknown opcode, a short frame, a wrong signature, an excessive count, an
// absent slot, or a list that the trusted packet size does not cover.
inline bool ReadDmgxSlot(const unsigned char* bytes, unsigned int packetSize,
    unsigned int opcode, int index, std::uint32_t& value)
{
    unsigned int baseSize = 0;
    if (opcode == MSG_Attack_One_Opcode)
        baseSize = static_cast<unsigned int>(kAttackOneBasePacketSize);
    else if (opcode == MSG_Attack_Two_Opcode)
        baseSize = static_cast<unsigned int>(kAttackTwoBasePacketSize);
    else if (opcode == MSG_Attack_Multi_Opcode)
        baseSize = static_cast<unsigned int>(kAttackMultiBasePacketSize);
    if (!bytes || index < 0 || !baseSize || packetSize < baseSize + 8)
        return false;

    std::uint32_t signature = 0;
    std::uint32_t count = 0;
    std::memcpy(&signature, bytes + baseSize, sizeof(signature));
    std::memcpy(&count, bytes + baseSize + 4, sizeof(count));
    if (signature != kAttackWideSignature || count > kAttackMultiTargetCapacity ||
        static_cast<unsigned int>(index) >= count ||
        packetSize < baseSize + 8 + count * sizeof(std::uint32_t))
        return false;

    std::memcpy(&value, bytes + baseSize + 8 + index * sizeof(std::uint32_t), sizeof(value));
    return true;
}

// Projection only, not a receive-frame validator. The caller owns the buffer
// and supplies its trusted header size; local previews may have a zero size.
// Keep negative legacy damage and malformed/unrepresentable tails unchanged.
// A08-02: a slot beyond the opcode's target capacity is not part of the frame
// (for example a flank slot 1 on a one-target 0x39D), so it projects no damage.
inline int ReadDamage(const unsigned char* bytes, unsigned int packetSize,
    unsigned int opcode, int index, int legacyDamage)
{
    if (!bytes || index < 0 || index >= TargetSlotLimit(opcode))
        return 0;
    if (legacyDamage < 0)
        return legacyDamage;

    std::uint32_t realDamage = 0;
    if (opcode == MSG_Attack_One_Opcode && packetSize == kAttackPhysicalWidePacketSize)
        std::memcpy(&realDamage, bytes + kAttackOneBasePacketSize, sizeof(realDamage));
    else if (!ReadDmgxSlot(bytes, packetSize, opcode, index, realDamage))
        return legacyDamage;

    if (realDamage > 0x7FFFFFFF)
        return legacyDamage;
    return static_cast<int>(realDamage);
}

// Heals travel as a negative prefix WORD (at most -32767). WYD-Go puts the
// real 32-bit heal in the same DMGX slot, so a heal reads that amount and
// returns it negated. Misses (-3/-4) and other sentinels above -6, local
// previews and absent or invalid tails keep the legacy value.
inline int ReadHeal(const unsigned char* bytes, unsigned int packetSize,
    unsigned int opcode, int index, int legacyDamage)
{
    if (legacyDamage > -6 || !bytes || index < 0 ||
        index >= static_cast<int>(kAttackMultiTargetCapacity))
        return legacyDamage;
    std::uint32_t realHeal = 0;
    if (!ReadDmgxSlot(bytes, packetSize, opcode, index, realHeal))
        return legacyDamage;
    if (realHeal == 0 || realHeal > 0x7FFFFFFF)
        return legacyDamage;
    return -static_cast<int>(realHeal);
}

// A08-03: WYD-Go sends a heal of h HP as the WORD -h with h in the DMGX slot,
// and a miss as the WORD -3 with a zero slot. Heals of 1..5 HP therefore share
// WORD values with the legacy sentinels above -6 (-3/-4 draw "miss"). Only a
// DMGX slot equal to the negated WORD identifies such a small heal. Frames
// without a valid tail (native servers, local previews) keep the sentinel.
inline bool IsTailConfirmedSmallHeal(const unsigned char* bytes, unsigned int packetSize,
    unsigned int opcode, int index, int legacyDamage)
{
    if (legacyDamage >= 0 || legacyDamage <= -6 || index >= TargetSlotLimit(opcode))
        return false;
    std::uint32_t tail = 0;
    return ReadDmgxSlot(bytes, packetSize, opcode, index, tail) &&
        tail == static_cast<std::uint32_t>(-legacyDamage);
}
}
