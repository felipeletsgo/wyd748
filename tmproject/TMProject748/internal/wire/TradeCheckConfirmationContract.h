#pragma once

#include "MessageHeader.h"
#include <cstddef>

// Server-to-client acknowledgement of the first trade check, without payload.
// Header.ID identifies the acknowledged human; the consumer only selects
// the local MyCheck control in the active trade window.
constexpr auto MSG_CNFTradeCheck_Opcode = 0x386;
constexpr std::size_t kTradeCheckConfirmationPacketSize = sizeof(MSG_STANDARD);

static_assert(kTradeCheckConfirmationPacketSize == 12,
    "Trade check confirmation must remain header-only");
