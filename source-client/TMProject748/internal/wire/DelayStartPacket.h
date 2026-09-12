#pragma once

#include "MessageHeader.h"
#include <cstddef>

// Intencao C->S sobrecarregada do 7.48. Parm zero cobre System/fechamento;
// valores 1 e 2 distinguem transicoes locais. O servidor nao confia no Parm.
constexpr auto MSG_DelayStart_Opcode = 0x3AE;
constexpr auto MSG_SysQuit_Opcode = MSG_DelayStart_Opcode;
struct MSG_DelayStart
{
	MSG_STANDARD Header;
	int Parm;
};

// O nome SysQuit documenta os callers de System sem duplicar o mesmo ABI.
using MSG_SysQuit = MSG_DelayStart;

static_assert(sizeof(MSG_DelayStart) == 16, "WYD 7.48 DelayStart packet must be 16 bytes");
static_assert(offsetof(MSG_DelayStart, Parm) == 12, "DelayStart parameter offset changed");
