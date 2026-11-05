#pragma once

#include "MessageHeader.h"

// C->S keepalive used in Field and SelectChar. The native frame contains only the header.
// Different IDs preserve the phase without adding a payload or response.
constexpr auto MSG_Ping_Opcode = 0x3A0;
static_assert(sizeof(MSG_STANDARD) == 12, "WYD 7.48 keepalive ping must remain 12 bytes");
