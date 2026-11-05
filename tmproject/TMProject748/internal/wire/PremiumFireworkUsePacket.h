#pragma once

#include "MessageHeader.h"
#include <cstddef>

// C->S drawable premium-firework intent. Parm carries 100 valid bits.
// The server validates item, position, reservation and cooldown.
constexpr auto MSG_UseItem2_Opcode = 0x3C9;
struct MSG_UseItem2
{
	MSG_STANDARD Header;
	int SourType;
	int SourPos;
	int DestType;
	int DestPos;
	unsigned short GridX;
	unsigned short GridY;
	unsigned short ItemID;
	char Parm[16];
};

static_assert(sizeof(MSG_UseItem2) == 52, "WYD 7.48 premium firework request must be 52 bytes");
static_assert(offsetof(MSG_UseItem2, SourType) == 12, "firework source type offset changed");
static_assert(offsetof(MSG_UseItem2, SourPos) == 16, "firework source position offset changed");
static_assert(offsetof(MSG_UseItem2, DestType) == 20, "firework destination type offset changed");
static_assert(offsetof(MSG_UseItem2, DestPos) == 24, "firework destination position offset changed");
static_assert(offsetof(MSG_UseItem2, GridX) == 28, "firework grid X offset changed");
static_assert(offsetof(MSG_UseItem2, GridY) == 30, "firework grid Y offset changed");
static_assert(offsetof(MSG_UseItem2, ItemID) == 32, "firework item ID offset changed");
static_assert(offsetof(MSG_UseItem2, Parm) == 34, "firework bitmap offset changed");
