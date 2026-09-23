#pragma once

#include <cstddef>

// Attacks retain three native 7.48 prefixes and accept only the coordinated
// tails published by WYD-Go. Validate the actual size before FieldScene reads
// the attacker, skill, or first damage entry.
constexpr auto MSG_Attack_Multi_Opcode = 0x36C;
constexpr auto MSG_Attack_One_Opcode = 0x39D;
constexpr auto MSG_Attack_Two_Opcode = 0x39E;

constexpr std::size_t kAttackAttackerIdOffset = 12;
constexpr std::size_t kAttackProgressOffset = 14;
constexpr std::size_t kAttackPositionOffset = 16;
constexpr std::size_t kAttackTargetPositionOffset = 20;
constexpr std::size_t kAttackSkillIndexOffset = 24;
constexpr std::size_t kAttackCurrentMpOffset = 26;
constexpr std::size_t kAttackMotionOffset = 28;
constexpr std::size_t kAttackSkillParameterOffset = 29;
constexpr std::size_t kAttackLocalFlagOffset = 30;
constexpr std::size_t kAttackDoubleCriticalOffset = 31;
constexpr std::size_t kAttackCurrentExpOffset = 32;
constexpr std::size_t kAttackRequiredMpOffset = 36;
constexpr std::size_t kAttackReservedOffset = 38;
constexpr std::size_t kAttackFakeExpOffset = 40;
constexpr std::size_t kAttackDamagesOffset = 44;
constexpr std::size_t kAttackDamageEntrySize = 4;

constexpr std::size_t kAttackOneTargetCapacity = 1;
constexpr std::size_t kAttackTwoTargetCapacity = 2;
constexpr std::size_t kAttackMultiTargetCapacity = 13;
constexpr std::size_t kAttackOneBasePacketSize = 48;
constexpr std::size_t kAttackTwoBasePacketSize = 52;
constexpr std::size_t kAttackMultiBasePacketSize = 96;

// PhysicalAttack appends an unsigned damage DWORD. SkillHits uses "DMGX",
// a count, and a DWORD list; the bounds reflect 1, 2, and 13 targets.
constexpr unsigned int kAttackWideSignature = 0x58474D44;
constexpr std::size_t kAttackPhysicalWidePacketSize = 52;
constexpr std::size_t kAttackOneSkillWidePacketSize = 60;
constexpr std::size_t kAttackTwoWideMinPacketSize = 64;
constexpr std::size_t kAttackTwoWideMaxPacketSize = 68;
constexpr std::size_t kAttackMultiWidePrefixSize = 104;
constexpr std::size_t kAttackMultiWideMinPacketSize = 108;
constexpr std::size_t kAttackMultiWideMaxPacketSize = 156;

constexpr bool IsAttackOpcode(unsigned int opcode)
{
    return opcode == MSG_Attack_One_Opcode ||
        opcode == MSG_Attack_Two_Opcode ||
        opcode == MSG_Attack_Multi_Opcode;
}

constexpr std::size_t AttackTargetCapacity(unsigned int opcode)
{
    switch (opcode)
    {
    case MSG_Attack_One_Opcode: return kAttackOneTargetCapacity;
    case MSG_Attack_Two_Opcode: return kAttackTwoTargetCapacity;
    case MSG_Attack_Multi_Opcode: return kAttackMultiTargetCapacity;
    default: return 0;
    }
}

// Client sends retain the native prefixes. The server also accepts One/96
// because that envelope was observed in the 7.48 client, but does not accept
// coordinated tail sizes used exclusively for server-to-client results.
constexpr bool IsClientToServerAttackPacketSize(unsigned int opcode, std::size_t size)
{
    switch (opcode)
    {
    case MSG_Attack_One_Opcode:
        return size == kAttackOneBasePacketSize ||
            size == kAttackMultiBasePacketSize;
    case MSG_Attack_Two_Opcode:
        return size == kAttackTwoBasePacketSize;
    case MSG_Attack_Multi_Opcode:
        return size == kAttackMultiBasePacketSize;
    default:
        return false;
    }
}

constexpr bool IsAttackPacketSize(unsigned int opcode, std::size_t size)
{
    switch (opcode)
    {
    case MSG_Attack_One_Opcode:
        return size == kAttackOneBasePacketSize ||
            size == kAttackPhysicalWidePacketSize ||
            size == kAttackOneSkillWidePacketSize;
    case MSG_Attack_Two_Opcode:
        return size == kAttackTwoBasePacketSize ||
            size == kAttackTwoWideMinPacketSize ||
            size == kAttackTwoWideMaxPacketSize;
    case MSG_Attack_Multi_Opcode:
        return size == kAttackMultiBasePacketSize ||
            (size >= kAttackMultiWideMinPacketSize &&
                size <= kAttackMultiWideMaxPacketSize &&
                (size - kAttackMultiWidePrefixSize) % kAttackDamageEntrySize == 0);
    default:
        return false;
    }
}
