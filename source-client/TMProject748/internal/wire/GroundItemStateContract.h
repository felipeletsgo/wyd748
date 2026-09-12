#pragma once

#include <cstddef>

// Frames S->C que atualizam ou removem um objeto do chao ja materializado.
constexpr auto MSG_UpdateItem_Opcode = 0x374;
constexpr std::size_t kGroundItemUpdatePacketSize = 20;
constexpr std::size_t kGroundItemUpdateItemIDOffset = 12;
constexpr std::size_t kGroundItemUpdateStateOffset = 16;
constexpr std::size_t kGroundItemUpdateHeightOffset = 18;

constexpr auto MSG_RemoveItem_Opcode = 0x16F;
constexpr std::size_t kGroundItemRemovePacketSize = 16;
constexpr std::size_t kGroundItemRemoveItemIDOffset = 12;
