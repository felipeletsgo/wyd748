#pragma once

#include <cstddef>

// Snapshot visual S->C do equipamento 7.48. Os 16 WORDs carregam item/refino
// visual; os 16 bytes finais carregam a cor antiga (AnctCode) por posicao.
constexpr auto MSG_UpdateEquip_Opcode = 0x36B;
constexpr std::size_t kUpdateEquipPacketSize = 60;
constexpr std::size_t kUpdateEquipVisualOffset = 12;
constexpr std::size_t kUpdateEquipSlotCount = 16;
constexpr std::size_t kUpdateEquipVisualSize = 2;
constexpr std::size_t kUpdateEquipAncientOffset = 44;
constexpr std::size_t kUpdateEquipAncientSize = 1;

