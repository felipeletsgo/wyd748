#pragma once

#include <cstddef>

// Ataques preservam tres prefixos nativos 7.48 e aceitam somente as caudas
// coordenadas que o WYD-Go publica. O tamanho real precisa ser validado antes
// que a FieldScene leia atacante, skill ou a primeira entrada de dano.
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

// PhysicalAttack acrescenta um DWORD de dano sem assinatura. SkillHits usa
// "DMGX", quantidade e uma lista DWORD; os limites refletem 1, 2 e 13 alvos.
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

// O envio do client preserva os prefixos nativos. O servidor tambem aceita
// One/96 porque esse envelope foi observado no client 7.48, mas nao aceita os
// tamanhos das caudas coordenadas usadas exclusivamente no sentido S->C.
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
