#pragma once

#include "MessageHeader.h"
#include <cstddef>

// Aviso textual S->C da cena base. O receptor usa somente frames com ID zero
// e encerra o corpo de 96 bytes antes de interpretar os prefixos do texto.
constexpr auto MSG_MessagePanel_Opcode = 0x101;
struct MSG_MessagePanel
{
	MSG_STANDARD Header;
	char String[96];
};

static_assert(sizeof(MSG_MessagePanel) == 108, "WYD 7.48 MSG_MessagePanel must be 108 bytes");
static_assert(offsetof(MSG_MessagePanel, String) == 12, "WYD 7.48 panel text offset changed");
