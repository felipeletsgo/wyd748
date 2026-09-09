#pragma once

#include "MessageHeader.h"
#include <cstddef>

// Intencao C->S do lider para remover um membro da guild. O servidor revalida
// cargo, alvo e persistencia; TargetID e somente a identidade solicitada.
constexpr auto MSG_GuildDeprivate_Opcode = 0x28C;
struct MSG_GuildDeprivate
{
    MSG_STANDARD Header;
    int TargetID;
};

static_assert(sizeof(MSG_GuildDeprivate) == 16, "WYD 7.48 guild deprivate must be 16 bytes");
static_assert(offsetof(MSG_GuildDeprivate, TargetID) == 12, "WYD 7.48 guild target offset changed");
