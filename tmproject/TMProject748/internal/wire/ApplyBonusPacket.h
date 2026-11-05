#pragma once

#include "MessageHeader.h"
#include <cstddef>

// C->S bonus/skill intent. The server validates BonusType, Detail and TargetID.
// The client does not publish skills, costs or prerequisites.
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
