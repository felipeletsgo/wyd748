#pragma once
#include "ChatMessagePacket.h"

// Coordinated S->C extension without a confirmed native 7.48 equivalent.
// Reuse only the envelope: ID=0, String[0]=0, int16 index at offset 14, optional CSV at offset 16.
// The scene parser validates contents and recipient.
constexpr auto MSG_MessageIndexed_Opcode = 0x105;
constexpr auto MSG_MessageParameterized_Opcode = 0x106;
constexpr std::size_t kIndexedMessagePacketSize = sizeof(MSG_MessageChat);
static_assert(kIndexedMessagePacketSize == 108, "Indexed message envelope changed");
