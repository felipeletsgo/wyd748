#pragma once

#include "MessageHeader.h"
#include <cstddef>

// Intencao/snapshot C<->S de remocao Party. Zero dissolve/limpa a lista;
// valor diferente de zero identifica o membro removido.
constexpr auto MSG_RemoveParty_Opcode = 0x37E;
struct MSG_RemoveParty
{
	MSG_STANDARD Header;
	int Parm;
};

static_assert(sizeof(MSG_RemoveParty) == 16, "WYD 7.48 party removal must be 16 bytes");
static_assert(offsetof(MSG_RemoveParty, Parm) == 12, "Party removal member offset changed");
