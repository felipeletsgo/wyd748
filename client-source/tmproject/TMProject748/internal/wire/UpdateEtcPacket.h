#pragma once

#include "MessageHeader.h"
#include <cstddef>
#include <cstdint>

// Snapshot compacto S->C. Hold representa divida de EXP; CP/Chaos nao ocupa
// este frame. Score completo continua pertencendo ao packet 0x336.
constexpr auto MSG_UpdateEtc_Opcode = 0x337;
struct MSG_UpdateEtc
{
	MSG_STANDARD Header;
	std::uint32_t Hold;
	std::uint32_t Exp;
	std::uint32_t LearnedSkill;
	std::uint16_t StatusPoint;
	std::uint16_t MasterPoint;
	std::uint16_t SkillPoint;
	std::uint16_t Magic;
	std::uint32_t Coin;
};

static_assert(sizeof(MSG_UpdateEtc) == 36, "WYD 7.48 UpdateEtc must be 36 bytes");
static_assert(offsetof(MSG_UpdateEtc, Hold) == 12, "UpdateEtc Hold offset changed");
static_assert(offsetof(MSG_UpdateEtc, Exp) == 16, "UpdateEtc EXP offset changed");
static_assert(offsetof(MSG_UpdateEtc, LearnedSkill) == 20, "UpdateEtc skill offset changed");
static_assert(offsetof(MSG_UpdateEtc, StatusPoint) == 24, "UpdateEtc status offset changed");
static_assert(offsetof(MSG_UpdateEtc, MasterPoint) == 26, "UpdateEtc mastery offset changed");
static_assert(offsetof(MSG_UpdateEtc, SkillPoint) == 28, "UpdateEtc skill-point offset changed");
static_assert(offsetof(MSG_UpdateEtc, Magic) == 30, "UpdateEtc magic offset changed");
static_assert(offsetof(MSG_UpdateEtc, Coin) == 32, "UpdateEtc coin offset changed");
