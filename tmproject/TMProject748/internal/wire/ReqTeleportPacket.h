#pragma once

#include "MessageHeader.h"
#include <cstddef>

// C->S intent to use the portal under the character. No destination or price is sent.
// The server resolves the route and charges authoritatively.
constexpr auto MSG_ReqTeleport_Opcode = 0x290;
struct MSG_ReqTeleport
{
    MSG_STANDARD Header;
    int Reserved;
};

static_assert(sizeof(MSG_ReqTeleport) == 16, "WYD 7.48 teleport request must be 16 bytes");
static_assert(offsetof(MSG_ReqTeleport, Reserved) == 12, "WYD 7.48 teleport reserved offset changed");
