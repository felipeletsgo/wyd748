#pragma once

#include "MessageHeader.h"
#include <cstddef>

// Overloaded 7.48 C->S intent. Parm zero covers System/close; values 1 and 2 distinguish local transitions.
// The server does not trust Parm.
constexpr auto MSG_DelayStart_Opcode = 0x3AE;
constexpr auto MSG_SysQuit_Opcode = MSG_DelayStart_Opcode;
struct MSG_DelayStart
{
	MSG_STANDARD Header;
	int Parm;
};

// SysQuit names the System callers without duplicating the ABI.
using MSG_SysQuit = MSG_DelayStart;

static_assert(sizeof(MSG_DelayStart) == 16, "WYD 7.48 DelayStart packet must be 16 bytes");
static_assert(offsetof(MSG_DelayStart, Parm) == 12, "DelayStart parameter offset changed");
