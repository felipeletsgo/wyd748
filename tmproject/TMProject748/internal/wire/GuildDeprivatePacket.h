#pragma once

#include "MessageHeader.h"
#include <cstddef>

// C->S leader intent to remove a guild member. The server validates rank, target and persistence.
// TargetID is only the requested identity.
constexpr auto MSG_GuildDeprivate_Opcode = 0x28C;
struct MSG_GuildDeprivate
{
    MSG_STANDARD Header;
    int TargetID;
};

static_assert(sizeof(MSG_GuildDeprivate) == 16, "WYD 7.48 guild deprivate must be 16 bytes");
static_assert(offsetof(MSG_GuildDeprivate, TargetID) == 12, "WYD 7.48 guild target offset changed");
