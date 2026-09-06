#pragma once
#include "MessageHeader.h"
#include <cstddef>

// Envelope bidirecional de whisper/canal/mail; prefixos definidos nos handlers.
// Nome e texto sao inline; receptor recebe buffer emprestado e gravavel.
constexpr auto MSG_MessageWhisper_Opcode = 0x334;
struct MSG_MessageWhisper
{
	MSG_STANDARD Header;
	char MobName[16];
	// WYD 7.48 uses a 96-byte body and a DWORD color, for 128 bytes total.
	// The imported 7.69 layout shifted Color and broke both commands and mail.
	char String[96];
	int Color;
};
static_assert(sizeof(MSG_MessageWhisper) == 128, "WYD 7.48 MSG_MessageWhisper must be 128 bytes");
static_assert(offsetof(MSG_MessageWhisper, MobName) == 12, "WYD 7.48 whisper name offset changed");
static_assert(offsetof(MSG_MessageWhisper, String) == 28, "WYD 7.48 whisper text offset changed");
static_assert(offsetof(MSG_MessageWhisper, Color) == 124, "WYD 7.48 whisper color offset changed");
