#include "../internal/ui/FieldChatControl.h"

#include <cstdio>
#include <vector>

namespace
{
struct Button
{
    int m_bSelected;
    int identifier;
    std::vector<int>* trace;
    void Update() { trace->push_back(identifier); }
};
}

int RunFieldChatControlTests(int& checks)
{
    using namespace field_chat;
    int failures = 0;
    const auto check = [&](bool passed, const char* message) {
        ++checks;
        if (!passed) {
            ++failures;
            std::fprintf(stderr, "FAIL field chat control: %s\n", message);
        }
    };
    const unsigned int controls[]{
        TMB_CHAT_GENERAL, TMB_CHAT_PARTY, TMB_CHAT_WHISPER, TMB_CHAT_GUILD,
        TMB_CHAT_GENERAL_T, TMB_CHAT_PARTY_T, TMB_CHAT_WHISPER_T, TMB_CHAT_GUILD_T};
    const Channel channels[]{
        Channel::General, Channel::Party, Channel::Whisper, Channel::Guild,
        Channel::General, Channel::Party, Channel::Whisper, Channel::Guild};
    for (int i = 0; i < 8; ++i)
    for (bool nativeHUD : {false, true})
    for (bool primaryPresent : {false, true})
    for (bool statePresent : {false, true})
    for (int selected : {-2, 0, 1})
    {
        std::vector<int> trace;
        Button primary{selected, 1, &trace}, state{7, 2, &trace};
        const ButtonPair<Button> pair{
            primaryPresent ? &primary : nullptr, statePresent ? &state : nullptr};
        const Buttons<Button> buttons{pair, pair, pair, pair};
        const auto expectedChannel = nativeHUD ? channels[i] : Channel::None;
        const auto expectedOutcome = !nativeHUD ? Outcome::Unhandled :
            !primaryPresent ? Outcome::Rejected : Outcome::Handled;
        const bool enabled = selected == 0;
        int callbacks = 0;
        Channel callbackChannel = Channel::None;
        bool callbackEnabled = false;
        const auto outcome = Handle(nativeHUD, controls[i], buttons,
            [&](Channel channel, bool value) {
                ++callbacks;
                callbackChannel = channel;
                callbackEnabled = value;
                trace.push_back(3);
                // Same flag/button capability as SetWhisper/SetPartyChat/SetGuildChat.
                primary.m_bSelected = value;
                primary.Update();
                if (statePresent) {
                    state.m_bSelected = value;
                    state.Update();
                }
            });
        check(Route(nativeHUD, controls[i]) == expectedChannel, "resource hit targets retain channel identity");
        check(outcome == expectedOutcome, "native rejection is consumed and imported input stays unhandled");
        const bool handled = expectedOutcome == Outcome::Handled;
        const bool channelFlag = handled && channels[i] != Channel::General;
        check(callbacks == (channelFlag ? 1 : 0) &&
            (!channelFlag || (callbackChannel == channels[i] && callbackEnabled == enabled)),
            "only the matching non-general channel flag is changed");
        check(primary.m_bSelected == (handled ? static_cast<int>(enabled) : selected),
            "both native hit targets toggle primary state; missing controls do not mutate it");
        check(state.m_bSelected == (handled && statePresent ? static_cast<int>(enabled) : 7),
            "a missing paired control remains optional and rejection leaves it unchanged");
        std::vector<int> expectedTrace;
        if (handled) {
            if (channelFlag) expectedTrace.push_back(3);
            expectedTrace.push_back(1);
            if (statePresent) expectedTrace.push_back(2);
        }
        check(trace == expectedTrace, "callback and primary/state update order is preserved exactly");
    }
    for (bool nativeHUD : {false, true})
    for (unsigned int control : {0u, static_cast<unsigned int>(TMB_CHAT_GENERAL - 1),
        static_cast<unsigned int>(TMB_CHAT_GUILD_T + 1), 0xFFFFFFFFu})
    {
        std::vector<int> trace;
        Button primary{0, 1, &trace}, state{1, 2, &trace};
        const ButtonPair<Button> pair{&primary, &state};
        const Buttons<Button> buttons{pair, pair, pair, pair};
        int callbacks = 0;
        check(Handle(nativeHUD, control, buttons,
            [&](Channel, bool) { ++callbacks; }) == Outcome::Unhandled,
            "out-of-domain controls are not swallowed");
        check(callbacks == 0 && trace.empty() && primary.m_bSelected == 0 && state.m_bSelected == 1,
            "out-of-domain routing has no writes, updates or callback");
    }
    check(Decide(Channel::None, true, 0).outcome == Outcome::Unhandled,
        "unhandled decisions cannot accidentally enable a channel");
    check(Decide(Channel::Unsupported, true, 0).outcome == Outcome::Rejected,
        "an unsupported native filter cannot fall through");
    // Distinct controls guard against a handler accidentally selecting a neighboring pair.
    std::vector<int> trace;
    Button general{0, 1, &trace}, party{1, 2, &trace}, whisper{1, 3, &trace}, guild{1, 4, &trace};
    const Buttons<Button> distinct{{&general, nullptr}, {&party, nullptr},
        {&whisper, nullptr}, {&guild, nullptr}};
    check(Handle(true, TMB_CHAT_GENERAL_T, distinct, [](Channel, bool) {}) == Outcome::Handled &&
        general.m_bSelected == 1 && party.m_bSelected == 1 && whisper.m_bSelected == 1 &&
        guild.m_bSelected == 1 && trace == std::vector<int>{1},
        "general filter touches only its borrowed primary control");
    return failures;
}
