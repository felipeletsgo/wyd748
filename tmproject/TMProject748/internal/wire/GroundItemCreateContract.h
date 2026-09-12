#pragma once

#include <cstddef>

// Snapshot S->C que materializa um item do chao no mundo 7.48.
constexpr auto MSG_CreateItem_Opcode = 0x26E;
constexpr std::size_t kGroundItemCreatePacketSize = 32;
constexpr std::size_t kGroundItemCreateGridXOffset = 12;
constexpr std::size_t kGroundItemCreateGridYOffset = 14;
constexpr std::size_t kGroundItemCreateItemIDOffset = 16;
constexpr std::size_t kGroundItemCreateItemOffset = 18;
constexpr std::size_t kGroundItemCreateRotateOffset = 26;
constexpr std::size_t kGroundItemCreateStateOffset = 27;
constexpr std::size_t kGroundItemCreateHeightOffset = 28;
constexpr std::size_t kGroundItemCreateFlagOffset = 29;
constexpr std::size_t kGroundItemCreateOwnerOffset = 30;
constexpr int kGroundItemDefinitionCount = 6500;

inline bool IsGroundItemDefinitionIndex(int itemIndex)
{
	return itemIndex > 0 && itemIndex < kGroundItemDefinitionCount;
}
