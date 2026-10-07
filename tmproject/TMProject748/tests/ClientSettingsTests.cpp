#include "../internal/core/ClientSettings.h"
#include <cstdio>

int RunClientSettingsTests(int& checks)
{
    int failures = 0;
    auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) { ++failures; std::printf("FAIL client settings: %s\n", name); }
    };

    ClientSettings defaults;
    check(defaults.focusedFps == 0 && defaults.idleFps == 15 && defaults.idleHideAmbient == 1 &&
        defaults.anisotropy == 4 && defaults.fieldCache == 4, "defaults match the approved plan");

    ClientSettings s;
    check(ParseClientSettingsLine("IdleFps=20\n", s) && s.idleFps == 20, "key=value is read");
    check(ParseClientSettingsLine("  FocusedFps = 144\r\n", s) && s.focusedFps == 144, "spaces and CRLF are accepted");
    check(!ParseClientSettingsLine("# IdleFps=30", s) && s.idleFps == 20, "comments are ignored");
    check(!ParseClientSettingsLine("Unknown=1", s), "unknown keys are ignored");
    check(!ParseClientSettingsLine("IdleFps=", s) && s.idleFps == 20, "a missing value keeps the previous one");
    ParseClientSettingsLine("IdleFps=1", s);
    check(s.idleFps == 5, "idle cap is clamped to at least 5 FPS");
    ParseClientSettingsLine("IdleFps=0", s);
    check(s.idleFps == 0, "zero disables the idle cap");
    ParseClientSettingsLine("Anisotropy=6", s);
    check(s.anisotropy == 4, "anisotropy rounds down to a supported level");
    ParseClientSettingsLine("FieldCache=99", s);
    check(s.fieldCache == 16, "field cache is bounded");

    check(EffectiveAnisotropy(8, 16, true) == 8 && EffectiveAnisotropy(16, 4, true) == 4 &&
        EffectiveAnisotropy(8, 16, false) == 0 && EffectiveAnisotropy(0, 16, true) == 0,
        "anisotropy follows the device limit and support");

    check(FrameWaitMs(1000.0, -1.0, 15) == 0.0, "the first frame runs at once");
    check(FrameWaitMs(1000.0, 990.0, 0) == 0.0, "no cap never waits");
    const double wait = FrameWaitMs(1000.0, 990.0, 15);
    check(wait > 56.6 && wait < 56.7, "15 FPS waits for the rest of the 66.7 ms period");
    check(FrameWaitMs(1100.0, 990.0, 15) == 0.0, "a late frame runs at once");

    g_ClientSettings = ClientSettings{};
    g_bIdleRenderProfile = true;
    check(IdleHidesAmbient(), "the idle profile hides ambient decoration by default");
    g_bIdleRenderProfile = false;
    check(!IdleHidesAmbient(), "the focused profile draws everything");
    return failures;
}
