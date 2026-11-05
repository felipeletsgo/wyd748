#pragma once

#include "MessageHeader.h"
#include <cstddef>
#include <cstdint>

// C->S guild-war/alliance intents. The client supplies local and target guilds.
// The server validates leadership and its own records before changing or persisting the relation.
constexpr auto MSG_GuildWar_Opcode = 0xE0E;
constexpr auto MSG_GuildAlly_Opcode = 0xE12;
struct MSG_GuildRelation
{
    MSG_STANDARD Header;
    std::uint32_t GuildID;
    std::uint32_t TargetGuildID;
};

static_assert(sizeof(MSG_GuildRelation) == 20, "WYD 7.48 guild relation request must be 20 bytes");
static_assert(offsetof(MSG_GuildRelation, GuildID) == 12, "WYD 7.48 guild relation source offset changed");
static_assert(offsetof(MSG_GuildRelation, TargetGuildID) == 16, "WYD 7.48 guild relation target offset changed");
