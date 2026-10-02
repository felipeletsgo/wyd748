#pragma once

#include "../core/ResourceControl.h"

namespace field_chat
{
enum class Channel { None, General, Party, Whisper, Guild, Unsupported };
enum class Outcome { Unhandled, Handled, Rejected };

struct Decision
{
    Outcome outcome;
    bool enabled;
};

// Both hit targets refer to the primary filter state. Imported HUD controls
// stay with the existing dispatcher; an unavailable native filter is consumed.
constexpr Channel Route(bool nativeHUD, unsigned int controlID)
{
    if (!nativeHUD || controlID < TMB_CHAT_GENERAL || controlID > TMB_CHAT_GUILD_T)
        return Channel::None;
    switch (controlID)
    {
    case TMB_CHAT_GENERAL: case TMB_CHAT_GENERAL_T: return Channel::General;
    case TMB_CHAT_PARTY: case TMB_CHAT_PARTY_T: return Channel::Party;
    case TMB_CHAT_WHISPER: case TMB_CHAT_WHISPER_T: return Channel::Whisper;
    case TMB_CHAT_GUILD: case TMB_CHAT_GUILD_T: return Channel::Guild;
    default: return Channel::Unsupported;
    }
}

constexpr Decision Decide(Channel channel, bool primaryPresent, int primarySelected)
{
    if (channel == Channel::None)
        return {Outcome::Unhandled, false};
    if (channel == Channel::Unsupported || !primaryPresent)
        return {Outcome::Rejected, false};
    return {Outcome::Handled, primarySelected == 0};
}
}
