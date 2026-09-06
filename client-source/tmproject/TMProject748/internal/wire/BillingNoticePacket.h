#pragma once

#include "MessageHeader.h"
#include <cstddef>

// Notificacao S->C de billing. O handler nativo apenas marca o estado e
// apresenta a mensagem; os quatro bytes apos o header permanecem opacos.
constexpr auto MSG_BillingNotice_Opcode = 0x194;
struct MSG_BillingNotice
{
	MSG_STANDARD Header;
	unsigned char OpaquePayload[4];
};

static_assert(sizeof(MSG_BillingNotice) == 16, "WYD 7.48 billing notice must be 16 bytes");
static_assert(offsetof(MSG_BillingNotice, OpaquePayload) == 12, "Billing payload offset changed");
