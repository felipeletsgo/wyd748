#pragma once
#include "MessageHeader.h"
#include <cstddef>

// Mensagens S->C opacas: a cena consome sem interpretar o payload.
constexpr auto MSG_LegacySceneMessage102_Opcode = 0x102;
struct MSG_LegacySceneMessage102
{
	MSG_STANDARD Header;
	unsigned char OpaquePayload[104];
};

constexpr auto MSG_LegacySceneMessage104_Opcode = 0x104;
struct MSG_LegacySceneMessage104
{
	MSG_STANDARD Header;
	unsigned char OpaquePayload[140];
};

static_assert(sizeof(MSG_LegacySceneMessage102) == 116, "WYD 7.48 legacy scene message 0x102 must be 116 bytes");
static_assert(offsetof(MSG_LegacySceneMessage102, OpaquePayload) == 12, "WYD 7.48 legacy scene message 0x102 payload offset changed");
static_assert(sizeof(MSG_LegacySceneMessage104) == 152, "WYD 7.48 legacy scene message 0x104 must be 152 bytes");
static_assert(offsetof(MSG_LegacySceneMessage104, OpaquePayload) == 12, "WYD 7.48 legacy scene message 0x104 payload offset changed");
