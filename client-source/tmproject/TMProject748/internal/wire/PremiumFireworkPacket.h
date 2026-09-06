#pragma once

#include "MessageHeader.h"
#include <cstddef>

// Publicacao S->C do desenho premium: primeiros 100 bits formam a grade 10x10.
constexpr auto MSG_PremiumFirework_Opcode = 0x3CA;
struct MSG_PremiumFirework
{
	MSG_STANDARD Header;
	char Reserved[8];
	char Bitmap[16];
};

static_assert(sizeof(MSG_PremiumFirework) == 36, "WYD 7.48 premium firework must be 36 bytes");
static_assert(offsetof(MSG_PremiumFirework, Reserved) == 12, "firework reserved offset changed");
static_assert(offsetof(MSG_PremiumFirework, Bitmap) == 20, "firework bitmap offset changed");
