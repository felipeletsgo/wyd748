#include "../internal/render/world/sky/LightingProfile.h"
#include <cstdio>
#include <filesystem>
#include <windows.h>

namespace {
std::filesystem::path FindLightingFile()
{
    wchar_t executable[MAX_PATH]{};
    const DWORD length = GetModuleFileNameW(nullptr, executable, MAX_PATH);
    if (length == 0 || length == MAX_PATH)
        return {};
    auto current = std::filesystem::path(executable).parent_path();
    for (int depth = 0; depth < 8 && !current.empty(); ++depth) {
        const auto candidate = current / "tmproject/client748/Env/Lighting.txt";
        if (std::filesystem::is_regular_file(candidate))
            return candidate;
        current = current.parent_path();
    }
    return {};
}

bool SameDefaults(const LightingProfile& a, const LightingProfile& b)
{
    for (int i = 0; i < 4; ++i) {
        if (a.sun[i].r != b.sun[i].r || a.sun[i].g != b.sun[i].g || a.sun[i].b != b.sun[i].b)
            return false;
        for (int c = 0; c < 3; ++c)
            if (a.sky[i][c] != b.sky[i][c])
                return false;
    }
    return a.fill.r == b.fill.r && a.AmbientColor() == b.AmbientColor();
}
}

int RunLightingProfileTests(int& checks)
{
    int failures = 0;
    auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) { ++failures; std::printf("FAIL lighting profile: %s\n", name); }
    };

    LightingProfile defaults;
    check(defaults.AmbientColor() == 0x33FFFFFFu, "default ambient keeps the previous render state");
    check(defaults.sky[0][0] == 28 && defaults.sky[0][1] == 120 && defaults.sky[0][2] == 189 &&
        defaults.sun[3].g == 0.34f, "default sky and sun keep the previous TMSky values");

    check(SkyStateIndex(67) == 0 && SkyStateIndex(70) == 3, "sky textures 67..70 map to states 0..3");
    check(SkyStateIndex(0) == 0 && SkyStateIndex(66) == 0 && SkyStateIndex(71) == 0 && SkyStateIndex(331) == 0,
        "other sky mesh textures stay inside the state tables");

    LightingProfile p;
    check(ParseLightingLine("Ambient=128 120 110\n", p) && p.AmbientColor() == 0x3380786Eu,
        "ambient bytes are read");
    check(ParseLightingLine("SunNight = 0.1 0.2 0.3\r\n", p) && p.sun[3].r == 0.1f && p.sun[3].b == 0.3f,
        "sun colors are read per state");
    check(ParseLightingLine("SkyRain=1 2 3", p) && p.sky[1][2] == 3, "sky colors are read per state");
    check(!ParseLightingLine("Ambient=300 0 0", p) && p.ambient[0] == 128, "out-of-range bytes are rejected");
    check(!ParseLightingLine("FillLight=0.5 0.5", p) && p.fill.r == 0.4f, "incomplete colors are rejected");
    check(!ParseLightingLine("# Ambient=0 0 0", p) && !ParseLightingLine("Unknown=1 1 1", p),
        "comments and unknown keys are ignored");

    const auto path = FindLightingFile();
    check(!path.empty(), "Env/Lighting.txt is shipped");
    if (!path.empty()) {
        const LightingProfile shipped = LoadLightingProfile(path.string().c_str());
        check(SameDefaults(shipped, LightingProfile{}), "the shipped file matches the defaults");
    }
    return failures;
}
