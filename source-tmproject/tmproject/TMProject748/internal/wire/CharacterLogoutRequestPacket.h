#pragma once

#include "MessageHeader.h"

// Intent C->S do logout: o client envia apenas o ID local apos o timer nativo.
// O servidor permanece autoritativo; este tipo nao e usado como resposta S->C.
constexpr auto MSG_CharacterLogout_Opcode = 0x215;
using MSG_CharacterLogout = MSG_STANDARD;

static_assert(sizeof(MSG_CharacterLogout) == 12,
	"WYD 7.48 character logout request must be 12 bytes");
