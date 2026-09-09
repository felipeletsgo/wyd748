#pragma once

#include "MessageHeader.h"

// Pedido C->S de renascimento/recall. O frame nativo e somente o header;
// score, posicao, HP e cooldown permanecem autoritativos no servidor.
constexpr auto MSG_Recall_Opcode = 0x289;
static_assert(sizeof(MSG_STANDARD) == 12, "WYD 7.48 restart/recall must remain 12 bytes");
