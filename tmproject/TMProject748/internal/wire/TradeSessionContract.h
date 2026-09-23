#pragma once

#include "MessageHeader.h"
#include <cstddef>

// Envelope bidirecional da sessao de trade usado pelo par TMProject748/WYD-Go.
// O payload completo de 0x383 e consumido como MSG_Trade; 0x384 apenas encerra
// a janela e, portanto, permanece um MSG_STANDARD sem payload.
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
