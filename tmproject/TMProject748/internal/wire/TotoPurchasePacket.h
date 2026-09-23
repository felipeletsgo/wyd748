#pragma once

#include "MessageHeader.h"
#include <cstddef>

// Client-to-server TOTO purchase intent; Coin remains a legacy,
// non-authoritative field.
constexpr auto MSG_BuyToto_Opcode = 0x3CE;
struct MSG_BuyToto
{
	MSG_STANDARD Header;
	unsigned short TargetID;
	short TargetCarryPos;
	short MyCarryPos;
	int Coin;
	int Gindex;
	int A_Score;
	int B_Score;
};

static_assert(sizeof(MSG_BuyToto) == 36, "WYD 7.48 TOTO packet must be 36 bytes");
static_assert(offsetof(MSG_BuyToto, TargetID) == 0x0C, "TOTO target offset changed");
static_assert(offsetof(MSG_BuyToto, TargetCarryPos) == 0x0E, "TOTO shop slot offset changed");
static_assert(offsetof(MSG_BuyToto, MyCarryPos) == 0x10, "TOTO carry slot offset changed");
static_assert(offsetof(MSG_BuyToto, Coin) == 0x14, "TOTO coin offset changed");
static_assert(offsetof(MSG_BuyToto, Gindex) == 0x18, "TOTO match offset changed");
static_assert(offsetof(MSG_BuyToto, A_Score) == 0x1C, "TOTO score A offset changed");
static_assert(offsetof(MSG_BuyToto, B_Score) == 0x20, "TOTO score B offset changed");
