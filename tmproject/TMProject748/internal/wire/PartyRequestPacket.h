#pragma once

#include "PartyAddPacket.h"
#include <cstddef>

// 7.48 client/server party invitation. Reuses the leader's PARTY snapshot and
// carries the target as an int32 at the end; the server revalidates both.
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
