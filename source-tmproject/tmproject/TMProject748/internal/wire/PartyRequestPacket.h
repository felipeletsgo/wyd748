#pragma once

#include "PartyAddPacket.h"
#include <cstddef>

// Convite Party C<->S do 7.48. Reutiliza o snapshot PARTY do líder e carrega
// o destino como int32 no fim do frame; o servidor revalida ambos.
constexpr auto MSG_REQParty_Opcode = 0x37F;
struct MSG_REQParty
{
	MSG_STANDARD Header;
	PARTY Leader;
	int TargetID;
};

static_assert(sizeof(MSG_REQParty) == 44, "WYD 7.48 party request must be 44 bytes");
static_assert(offsetof(MSG_REQParty, Leader) == 12, "WYD 7.48 party leader offset changed");
static_assert(offsetof(MSG_REQParty, TargetID) == 40, "WYD 7.48 party target offset changed");
