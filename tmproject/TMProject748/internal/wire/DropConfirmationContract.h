#pragma once

#include <cstddef>

// Confirmacao S->C do drop 7.48: origem, rotacao e coordenadas do item no chao.
constexpr auto MSG_CNFDropItem_Opcode = 0x175;
constexpr std::size_t kDropConfirmationPacketSize = 28;
constexpr std::size_t kDropConfirmationSourceTypeOffset = 12;
constexpr std::size_t kDropConfirmationSourcePosOffset = 16;
constexpr std::size_t kDropConfirmationRotateOffset = 20;
constexpr std::size_t kDropConfirmationGridXOffset = 24;
constexpr std::size_t kDropConfirmationGridYOffset = 26;
constexpr int kDropVisibleCarrySlotCount = 63;
constexpr int kDropUsableCargoSlotCount = 120;

inline bool IsDropCarrySlot(int slot)
{
	return slot >= 0 && slot < kDropVisibleCarrySlotCount;
}

inline bool IsDropCargoSlot(int slot)
{
	return slot >= 0 && slot < kDropUsableCargoSlotCount;
}
