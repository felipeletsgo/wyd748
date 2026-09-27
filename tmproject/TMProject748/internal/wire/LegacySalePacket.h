#pragma once

#include "MessageHeader.h"
#include <cstddef>

// Existing request/legacy-handler representation, not proof of a native
// S->C response contract. WYD-Go replies with SendItem and UpdateEtc instead.
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
