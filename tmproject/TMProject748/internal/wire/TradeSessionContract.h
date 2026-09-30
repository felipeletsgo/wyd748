#pragma once

#include "MessageHeader.h"
#include <cstddef>

// Bidirectional trade-session envelopes used by TMProject748/WYD-Go.
// Native incoming consumers use the complete 0x383 offer and header-only 0x384
// closure. The active pair also uses these layouts for outgoing intentions.
constexpr auto MSG_Trade_Opcode = 0x383;
constexpr auto MSG_CloseTrade_Opcode = 0x384;

constexpr std::size_t kTradePacketSize = 156;
constexpr std::size_t kCloseTradePacketSize = sizeof(MSG_STANDARD);

constexpr std::size_t kTradeItemsOffset = 12;
constexpr std::size_t kTradeCarryPositionsOffset = 132;
constexpr std::size_t kTradeMoneyOffset = 148;
constexpr std::size_t kTradeCheckOffset = 152;
constexpr std::size_t kTradeOpponentIdOffset = 154;

static_assert(kCloseTradePacketSize == 12,
    "CloseTrade must remain header-only");
static_assert(kTradeOpponentIdOffset + sizeof(unsigned short) == kTradePacketSize,
    "Trade opponent id must end the packet");
