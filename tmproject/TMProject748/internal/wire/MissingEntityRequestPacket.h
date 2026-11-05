#pragma once

#include "MessageHeader.h"
#include <cstddef>

// C->S intent requests an entity referenced by Action/Attack.
// The final WORD remains zero, as the native frame is zero-initialized.
constexpr auto MSG_REQMobByID_Opcode = 0x369;
struct MSG_REQMobByID
{
	MSG_STANDARD Header;
	short MobID;
	short Reserved;
};

static_assert(sizeof(MSG_REQMobByID) == 16, "WYD 7.48 missing-entity request must be 16 bytes");
static_assert(offsetof(MSG_REQMobByID, MobID) == 12, "WYD 7.48 missing-entity ID offset changed");
static_assert(offsetof(MSG_REQMobByID, Reserved) == 14, "WYD 7.48 missing-entity reserved offset changed");
