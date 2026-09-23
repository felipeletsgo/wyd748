#pragma once

#include <cstddef>

constexpr auto MSG_AutoTrade_Opcode = 0x397;
constexpr std::size_t kAutoTradePacketSize = 196;
constexpr std::size_t kAutoTradeDescriptionOffset = 12;
constexpr std::size_t kAutoTradeDescriptionSize = 24;
constexpr std::size_t kAutoTradeItemsOffset = 36;
constexpr std::size_t kAutoTradeItemCount = 12;
constexpr std::size_t kAutoTradeItemSize = 8;
constexpr std::size_t kAutoTradeCarryPositionsOffset = 132;
constexpr std::size_t kAutoTradePricesOffset = 144;
constexpr std::size_t kAutoTradePriceSize = 4;
constexpr std::size_t kAutoTradeTaxOffset = 192;
constexpr std::size_t kAutoTradeTargetIdOffset = 194;
