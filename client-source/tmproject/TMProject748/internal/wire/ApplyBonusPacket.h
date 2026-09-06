#pragma once

#include "MessageHeader.h"
#include <cstddef>

// Intent C->S de bonus/skill. BonusType, Detail e TargetID sao revalidados
// pelo servidor; o client nao publica skill, custo ou pre-requisitos.
constexpr auto MSG_ApplyBonus_Opcode = 0x277;
struct MSG_ApplyBonus
{
	MSG_STANDARD Header;
	short BonusType;
	short Detail;
	unsigned short TargetID;
};

static_assert(sizeof(MSG_ApplyBonus) == 20, "WYD 7.48 ApplyBonus must be 20 bytes");
static_assert(offsetof(MSG_ApplyBonus, BonusType) == 12, "ApplyBonus type offset changed");
static_assert(offsetof(MSG_ApplyBonus, Detail) == 14, "ApplyBonus detail offset changed");
static_assert(offsetof(MSG_ApplyBonus, TargetID) == 16, "ApplyBonus target offset changed");
