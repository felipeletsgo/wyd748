#pragma once

#include "MessageHeader.h"
#include <cstddef>

// S->C character-logout confirmation. Native 7.48 reads only the header and validates ID
// before copying Score/Equip and entering SelectChar.
constexpr auto MSG_CNFCharacterLogout_Opcode = 0x116;
using MSG_CNFCharacterLogout = MSG_STANDARD;

static_assert(sizeof(MSG_CNFCharacterLogout) == 12,
	"WYD 7.48 character logout confirmation must be 12 bytes");
