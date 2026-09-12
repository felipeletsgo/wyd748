#pragma once
#include "MessageHeader.h"
#include <cstddef>

// Chat local C->S/S->C: header de 12 bytes e texto de 96 bytes.
// O receptor empresta o frame gravavel e termina o texto antes de exibi-lo.
constexpr auto MSG_MessageChat_Opcode = 0x333;
struct MSG_MessageChat
{
	MSG_STANDARD Header;
	// Native 7.48 uses the same 96-byte text body in both directions.
	char String[96];
};
static_assert(sizeof(MSG_MessageChat) == 108, "WYD 7.48 MSG_MessageChat must be 108 bytes");
static_assert(offsetof(MSG_MessageChat, String) == 12, "WYD 7.48 chat text offset changed");
