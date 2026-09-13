#pragma once

#include "MessageHeader.h"

// Aviso S->C que abre a janela nativa de confirmacao da disputa de cidade.
// O client 7.48 recebe somente o MSG_STANDARD; alvo, cidade e cobranca ficam
// no contexto autoritativo do servidor e na confirmacao C->S subsequente.
constexpr auto MSG_ReqChallenge_Opcode = 0x18D;

static_assert(sizeof(MSG_STANDARD) == 12, "Guild challenge prompt must be header-only");
