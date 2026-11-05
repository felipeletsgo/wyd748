#pragma once

#include "MessageHeader.h"
#include <cstddef>

// C->S PK-toggle intent. The server accepts only Parm 0 or 1.
constexpr auto MSG_SetPKMode_Opcode = 0x399;
struct MSG_SetPKMode
{
	MSG_STANDARD Header;
	int Parm;
};

static_assert(sizeof(MSG_SetPKMode) == 16, "WYD 7.48 PK mode packet must be 16 bytes");
static_assert(offsetof(MSG_SetPKMode, Parm) == 12, "PK mode value offset changed");
