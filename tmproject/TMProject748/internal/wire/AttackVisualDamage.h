#pragma once

#include "AttackFrameContract.h"
#include <cstdint>
#include <cstring>

namespace attack_visual
{
// Projection only, not a receive-frame validator. The caller owns the buffer
// and supplies its trusted header size; local previews may have a zero size.
// Keep negative legacy damage and malformed/unrepresentable tails unchanged.
inline int ReadDamage(const unsigned char* bytes, unsigned int packetSize,
    unsigned int opcode, int index, int legacyDamage)
{
    if (!bytes || index < 0 || index >= static_cast<int>(kAttackMultiTargetCapacity))
        return 0;
    if (legacyDamage < 0)
        return legacyDamage;

    std::uint32_t realDamage = 0;
    if (opcode == MSG_Attack_One_Opcode && packetSize == kAttackPhysicalWidePacketSize) {
        std::memcpy(&realDamage, bytes + kAttackOneBasePacketSize, sizeof(realDamage));
    } else {
        unsigned int baseSize = 0;
        if (opcode == MSG_Attack_One_Opcode)
            baseSize = static_cast<unsigned int>(kAttackOneBasePacketSize);
        else if (opcode == MSG_Attack_Two_Opcode)
            baseSize = static_cast<unsigned int>(kAttackTwoBasePacketSize);
        else if (opcode == MSG_Attack_Multi_Opcode)
            baseSize = static_cast<unsigned int>(kAttackMultiBasePacketSize);

        if (!baseSize || packetSize < baseSize + 8)
            return legacyDamage;

        std::uint32_t signature = 0;
        std::uint32_t count = 0;
        std::memcpy(&signature, bytes + baseSize, sizeof(signature));
        std::memcpy(&count, bytes + baseSize + 4, sizeof(count));
        if (signature != kAttackWideSignature || count > kAttackMultiTargetCapacity ||
            static_cast<unsigned int>(index) >= count ||
            packetSize < baseSize + 8 + count * sizeof(std::uint32_t))
            return legacyDamage;

        std::memcpy(&realDamage, bytes + baseSize + 8 + index * sizeof(std::uint32_t),
            sizeof(realDamage));
    }

    if (realDamage > 0x7FFFFFFF)
        return legacyDamage;
    return static_cast<int>(realDamage);
}
}
