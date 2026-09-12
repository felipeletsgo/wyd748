#pragma once

#include "MessageHeader.h"
#include <cstddef>

// Intent C->S de alternancia PK; Parm aceita somente 0 ou 1 no servidor.
constexpr auto MSG_SetPKMode_Opcode = 0x399;
struct MSG_SetPKMode
{
	MSG_STANDARD Header;
	int Parm;
};

static_assert(sizeof(MSG_SetPKMode) == 16, "WYD 7.48 PK mode packet must be 16 bytes");
static_assert(offsetof(MSG_SetPKMode, Parm) == 12, "PK mode value offset changed");
