#pragma once

#include "MessageHeader.h"
#include <cstddef>

// Base-scene S->C notice. Accept only ID zero and terminate the 96-byte body
// before interpreting text prefixes.
constexpr auto MSG_MessagePanel_Opcode = 0x101;
struct MSG_MessagePanel
{
	MSG_STANDARD Header;
	char String[96];
};

static_assert(sizeof(MSG_MessagePanel) == 108, "WYD 7.48 MSG_MessagePanel must be 108 bytes");
static_assert(offsetof(MSG_MessagePanel, String) == 12, "WYD 7.48 panel text offset changed");
