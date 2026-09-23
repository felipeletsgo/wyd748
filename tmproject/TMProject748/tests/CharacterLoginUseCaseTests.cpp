#include "../internal/application/RequestCharacterLogin.h"
#include <climits>
#include <initializer_list>
#include <cstdio>

namespace
{
// Wire-free test double: accept any index so the encoder's additional guard
// cannot hide a regression in use-case validation.
struct RecordingLoginSender final : ICharacterLoginSender
{
    int calls = 0;
    int lastSlot = -1;
    bool result = true;

    // Record the received intent and return the configured local result.
    bool SendCharacterLogin(int slot) override
    {
        ++calls;
        lastSlot = slot;
        return result;
    }
};
}

// Separate unit: compile the application without wire, socket, or Basedef.
// Add checks to the shared runner; they remain active with NDEBUG in Release.
int RunCharacterLoginUseCaseTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) {
            std::fprintf(stderr, "FAIL: %s\n", name);
            ++failures;
        }
    };
    RecordingLoginSender sender;
    for (int slot = 0; slot < 4; ++slot) {
        check(RequestCharacterLogin(sender, slot), "valid login intent accepted");
        check(sender.calls == slot + 1 && sender.lastSlot == slot,
            "semantic port receives the slot once");
    }
    for (int slot : {INT_MIN, -1, 4, INT_MAX}) {
        check(!RequestCharacterLogin(sender, slot), "invalid login intent rejected");
    }
    check(sender.calls == 4, "use case blocks invalid slots before the encoder");
    sender.result = false;
    check(!RequestCharacterLogin(sender, 2), "semantic failure propagated");
    check(sender.calls == 5 && sender.lastSlot == 2, "semantic failure does not retry");
    return failures;
}
