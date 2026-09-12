#pragma once

#include "MessageHeader.h"
#include <cstddef>
#include <cstdint>

// Snapshot S->C da morte: Hold/FakeExp e EXP pertencem ao destinatario
// indicado pelo fluxo; o client apenas projeta estado persistido pelo servidor.
constexpr auto MSG_CNFMobKill_Opcode = 0x338;
struct MSG_CNFMobKill
{
	MSG_STANDARD Header;
	std::uint32_t FakeExp;
	std::uint16_t KilledMob;
	std::uint16_t Killer;
	std::uint32_t Exp;
};

static_assert(sizeof(MSG_CNFMobKill) == 24, "WYD 7.48 mob-kill confirmation must be 24 bytes");
static_assert(offsetof(MSG_CNFMobKill, FakeExp) == 12, "mob-kill Hold offset changed");
static_assert(offsetof(MSG_CNFMobKill, KilledMob) == 16, "mob-kill victim offset changed");
static_assert(offsetof(MSG_CNFMobKill, Killer) == 18, "mob-kill killer offset changed");
static_assert(offsetof(MSG_CNFMobKill, Exp) == 20, "mob-kill EXP offset changed");
