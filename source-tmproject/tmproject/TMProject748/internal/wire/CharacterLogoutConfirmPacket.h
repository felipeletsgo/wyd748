#pragma once

#include "MessageHeader.h"
#include <cstddef>

// Confirmacao S->C do logout de personagem. O nativo 7.48 consome somente
// o header e valida o ID antes de copiar Score/Equip e mudar para SelectChar.
constexpr auto MSG_CNFCharacterLogout_Opcode = 0x116;
using MSG_CNFCharacterLogout = MSG_STANDARD;

static_assert(sizeof(MSG_CNFCharacterLogout) == 12,
	"WYD 7.48 character logout confirmation must be 12 bytes");
