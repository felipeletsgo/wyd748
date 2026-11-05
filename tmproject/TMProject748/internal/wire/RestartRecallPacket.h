#pragma once

#include "MessageHeader.h"

// C->S respawn/recall request. The native frame contains only the header.
// The server owns score, position, HP and cooldown.
constexpr auto MSG_Recall_Opcode = 0x289;
static_assert(sizeof(MSG_STANDARD) == 12, "WYD 7.48 restart/recall must remain 12 bytes");
