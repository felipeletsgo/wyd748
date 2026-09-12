#pragma once

#include "MessageHeader.h"

// Keepalive C->S usado em Field e SelectChar. O frame nativo e somente o
// header; a diferenca de ID preserva a fase sem criar payload ou resposta.
constexpr auto MSG_Ping_Opcode = 0x3A0;
static_assert(sizeof(MSG_STANDARD) == 12, "WYD 7.48 keepalive ping must remain 12 bytes");
