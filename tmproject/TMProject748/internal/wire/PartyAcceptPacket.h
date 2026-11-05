#pragma once

#include "MessageHeader.h"
#include <cstddef>

// 7.48 C->S party acceptance intent. The server validates the leader and name.
// The final native WORD remains reserved.
constexpr auto MSG_CNFParty2_Opcode = 0x3AB;
struct MSG_CNFParty2
{
	MSG_STANDARD Header;
	short LeaderID;
	char LeaderName[16];
	short Reserved;
};

static_assert(sizeof(MSG_CNFParty2) == 32, "WYD 7.48 party acceptance must be 32 bytes");
static_assert(offsetof(MSG_CNFParty2, LeaderID) == 12, "WYD 7.48 party leader ID offset changed");
static_assert(offsetof(MSG_CNFParty2, LeaderName) == 14, "WYD 7.48 party leader name offset changed");
static_assert(offsetof(MSG_CNFParty2, Reserved) == 30, "WYD 7.48 party reserved offset changed");
