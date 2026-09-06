#pragma once

#include "MessageHeader.h"
#include <cstddef>

// Frame C<->S do 7.48 para motions e efeitos pontuais. Emotes enviados pelo
// jogador usam Parm/Direction zero; efeitos especiais sao publicados pelo servidor.
constexpr auto MSG_Motion_Opcode = 0x36A;
struct MSG_Motion
{
	MSG_STANDARD Header;
	short Motion;
	short Parm;
	float Direction;
};

static_assert(sizeof(MSG_Motion) == 20, "WYD 7.48 motion packet must be 20 bytes");
static_assert(offsetof(MSG_Motion, Motion) == 12, "WYD 7.48 motion offset changed");
static_assert(offsetof(MSG_Motion, Parm) == 14, "WYD 7.48 motion parameter offset changed");
static_assert(offsetof(MSG_Motion, Direction) == 16, "WYD 7.48 motion direction offset changed");
