#pragma once

#include <cstddef>

// Confirmacao S->C da coleta 7.48: header, destino e snapshot de oito bytes.
// A struct concreta permanece em Basedef porque depende de STRUCT_ITEM.
constexpr auto MSG_CNFGetItem_Opcode = 0x171;
constexpr std::size_t kPickupConfirmationPacketSize = 28;
constexpr std::size_t kPickupConfirmationDestTypeOffset = 12;
constexpr std::size_t kPickupConfirmationDestPosOffset = 16;
constexpr std::size_t kPickupConfirmationItemOffset = 20;
constexpr int kPickupVisibleCarrySlotCount = 63;

inline bool IsPickupCarrySlot(int slot)
{
	return slot >= 0 && slot < kPickupVisibleCarrySlotCount;
}
