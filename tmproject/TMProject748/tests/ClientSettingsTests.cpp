#include "../internal/core/ClientSettings.h"
#include <cstdio>
#include <initializer_list>

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
    for (const char* invalid : {"IdleFps=20fps", "IdleFps=20.5", "IdleFps=999999999999999999999",
        "IdleFps=-999999999999999999999", "IdleFps=20 IdleFps=30"})
        check(!ParseClientSettingsLine(invalid, s) && s.idleFps == 20,
            "invalid integers preserve the previous setting");
    check(ParseClientSettingsLine("IdleFps=25 # light profile\r\n", s) && s.idleFps == 25,
        "integer values allow trailing comments");
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

    ClientSettings scale;
    check(scale.uiScale == 0.0f, "the interface scale is automatic by default");
    check(ParseClientSettingsLine("UIScale=1.5", scale) && scale.uiScale == 1.5f, "a fixed scale is read");
    for (const char* invalid : {"UIScale=Au", "UIScale=Aut", "UIScale=Automatic", "UIScale=Autojunk",
        "UIScale=nan", "UIScale=-nan", "UIScale=inf", "UIScale=-inf", "UIScale=1e99",
        "UIScale=1e-99", "UIScale=1.5px", "UIScale=1.5 2", "UIScale=", "UIScale=A"})
        check(!ParseClientSettingsLine(invalid, scale) && scale.uiScale == 1.5f,
            "invalid scale tokens preserve the previous setting");
    check(ParseClientSettingsLine("UIScale=1.25 # uniform\r\n", scale) && scale.uiScale == 1.25f,
        "numeric scales allow trailing comments");
    check(ParseClientSettingsLine("UIScale=aUtO\t# resolution\r\n", scale) && scale.uiScale == 0.0f,
        "the complete Auto token is case insensitive and allows comments");
    check(ParseClientSettingsLine("UIScale=Auto", scale) && scale.uiScale == 0.0f, "Auto selects the automatic scale");
    ParseClientSettingsLine("UIScale=9", scale);
    check(scale.uiScale == 4.0f, "the fixed scale is bounded");
    check(ResolveUIScale(0.0f, 800, 600) == 1.0f && ResolveUIScale(0.0f, 640, 480) == 1.0f,
        "800x600 and smaller keep the native size");
    check(ResolveUIScale(0.0f, 1024, 768) == 1.25f, "1024x768 scales by 1.25 (1.28 rounded down)");
    check(ResolveUIScale(0.0f, 1280, 1024) == 1.6f, "1280x1024 is limited by its width");
    check(ResolveUIScale(0.0f, 1920, 1080) == 1.8f, "1920x1080 is limited by its height");
    check(ResolveUIScale(1.5f, 1920, 1080) == 1.5f, "a fixed scale overrides the automatic one");
    check(ScaledFontSize(1.0f) == 13 && ScaledFontSize(1.8f) == 23, "the font follows the scale");
    check(FontTextureSizeFor(13) == 512 && FontTextureSizeFor(23) == 1024,
        "larger fonts use a wider glyph texture");

    ClientSettings weather;
    check(weather.weatherArea == 12, "rain and snow cover a 24x24 area by default");
    check(ParseClientSettingsLine("WeatherArea=3", weather) && weather.weatherArea == 6,
        "the weather area never shrinks below the classic size");

    g_ClientSettings = ClientSettings{};
    g_bIdleRenderProfile = true;
    check(IdleHidesAmbient(), "the idle profile hides ambient decoration by default");
    g_bIdleRenderProfile = false;
    check(!IdleHidesAmbient(), "the focused profile draws everything");
    return failures;
}
