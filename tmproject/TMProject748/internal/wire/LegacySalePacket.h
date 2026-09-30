#pragma once

#include "MessageHeader.h"
#include <cstddef>

// Native 7.48 sale representation: the received 0x37A envelope is exactly
// 20 bytes. WYD-Go uses this opcode for the request, but replies with
// authoritative SendItem and UpdateEtc snapshots instead.
constexpr auto MSG_Sell_Opcode = 0x37A;
struct MSG_Sell
{
	MSG_STANDARD Header;
	unsigned short TargetID;
	short MyType;
	short MyPos;
};

static_assert(sizeof(MSG_Sell) == 20, "Legacy sale representation changed");
static_assert(offsetof(MSG_Sell, TargetID) == 12, "Legacy sale merchant offset changed");
static_assert(offsetof(MSG_Sell, MyType) == 14, "Legacy sale slot type offset changed");
static_assert(offsetof(MSG_Sell, MyPos) == 16, "Legacy sale slot position offset changed");
