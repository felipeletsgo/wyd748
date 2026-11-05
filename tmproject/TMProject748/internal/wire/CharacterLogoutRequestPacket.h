#pragma once

#include "MessageHeader.h"

// C->S logout intent: send only the local ID after the native timer.
// The server remains authoritative. This type is not an S->C response.
constexpr auto MSG_CharacterLogout_Opcode = 0x215;
using MSG_CharacterLogout = MSG_STANDARD;

static_assert(sizeof(MSG_CharacterLogout) == 12,
	"WYD 7.48 character logout request must be 12 bytes");
