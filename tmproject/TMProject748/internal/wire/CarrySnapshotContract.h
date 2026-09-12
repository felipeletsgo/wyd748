#pragma once

#include <cstddef>

// Snapshot S->C do Carry 7.48. O wire conserva 64 STRUCT_ITEM; somente os 63
// primeiros possuem celula visual na grade 9x7 do jogador.
constexpr auto MSG_UpdateCarry_Opcode = 0x185;
constexpr std::size_t kCarrySnapshotPacketSize = 528;
constexpr std::size_t kCarrySnapshotItemsOffset = 12;
constexpr std::size_t kCarrySnapshotItemCount = 64;
constexpr std::size_t kCarrySnapshotItemSize = 8;
constexpr std::size_t kCarrySnapshotCoinOffset = 524;
constexpr std::size_t kCarrySnapshotVisibleItemCount = 63;

