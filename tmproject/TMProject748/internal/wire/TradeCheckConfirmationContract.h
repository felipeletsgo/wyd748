#pragma once

#include "MessageHeader.h"
#include <cstddef>

// Confirmacao S->C do primeiro check do trade. O evento nao possui payload:
// a identidade do personagem permanece no Header.ID e o consumidor apenas
// marca o controle MyCheck da janela ativa.
constexpr auto MSG_CNFTradeCheck_Opcode = 0x386;
constexpr std::size_t kTradeCheckConfirmationPacketSize = sizeof(MSG_STANDARD);

static_assert(kTradeCheckConfirmationPacketSize == 12,
    "Trade check confirmation must remain header-only");
