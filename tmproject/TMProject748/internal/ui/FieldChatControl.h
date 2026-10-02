#pragma once

#include "../application/FieldChatControlPolicy.h"

namespace field_chat
{
// A synchronous borrowed view, not a container or a new UI owner. Never retain
// controls, callbacks or selections beyond the current control event.
template <typename Button>
struct ButtonPair
{
    Button* primary;
    Button* state;
};

template <typename Button>
struct Buttons
{
    ButtonPair<Button> general, party, whisper, guild;
};

// The only scene capability supplied is setting one channel flag through its
// existing member method. No scene pointer, packet buffer or global lookup.
template <typename Button, typename SetChannel>
Outcome Handle(bool nativeHUD, unsigned int controlID, const Buttons<Button>& buttons,
    SetChannel&& setChannel)
{
    const auto channel = Route(nativeHUD, controlID);
    if (channel == Channel::None)
        return Outcome::Unhandled;
    ButtonPair<Button> pair{};
    switch (channel)
    {
    case Channel::General: pair = buttons.general; break;
    case Channel::Party: pair = buttons.party; break;
    case Channel::Whisper: pair = buttons.whisper; break;
    case Channel::Guild: pair = buttons.guild; break;
    default: return Outcome::Rejected;
    }
    const auto decision = Decide(channel, pair.primary != nullptr,
        pair.primary ? pair.primary->m_bSelected : 0);
    if (decision.outcome != Outcome::Handled)
        return decision.outcome;
    if (channel != Channel::General)
    {
        setChannel(channel, decision.enabled);
        return Outcome::Handled;
    }
    // Preserve write/write/update/update ordering and optional paired controls.
    pair.primary->m_bSelected = decision.enabled;
    if (pair.state)
        pair.state->m_bSelected = decision.enabled;
    pair.primary->Update();
    if (pair.state)
        pair.state->Update();
    return Outcome::Handled;
}
}
