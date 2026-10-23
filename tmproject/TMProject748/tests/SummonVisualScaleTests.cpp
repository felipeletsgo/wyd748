#include "../internal/game/entities/MonsterVisualScale.h"
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <windows.h>

// Server companions ("Name^") copy the caster's CON; their size must stay at
// the base scale instead of following CON like ordinary monsters.
namespace
{
std::string ReadAppearanceSource()
{
    wchar_t executable[MAX_PATH]{};
    const auto length = GetModuleFileNameW(nullptr, executable, MAX_PATH);
    if (!length || length == MAX_PATH) return {};
    auto directory = std::filesystem::path(executable).parent_path();
    for (int depth = 0; depth < 8 && !directory.empty(); ++depth) {
        std::ifstream input(directory / "tmproject/TMProject748/internal/game/entities/TMHumanAppearance.cpp", std::ios::binary);
        if (input) return {std::istreambuf_iterator<char>(input), {}};
        directory = directory.parent_path();
    }
    return {};
}
}

int RunSummonVisualScaleTests(int& checks)
{
    int failures = 0;
    auto check = [&](bool condition, const char* message) {
        ++checks;
        if (!condition) {
            ++failures;
            std::printf("FAIL: %s\n", message);
        }
    };
    using monster_visual_scale::IsSummonName;
    check(IsSummonName("Condor^") && IsSummonName("Hatchling^") && IsSummonName("Wolf^x"),
        "server companion names are summons");
    check(!IsSummonName("Condor") && !IsSummonName("^Condor") && !IsSummonName("") && !IsSummonName(nullptr),
        "ordinary or malformed names are not summons");

    const auto source = ReadAppearanceSource();
    check(!source.empty(), "appearance source located");
    const auto body = source.find("void TMHuman::SetCharHeight(float fCon)");
    const auto summon = source.find("monster_visual_scale::IsSummonName(m_szName)", body);
    const auto base = source.find("m_fScale = monster_visual_scale::Base;", summon);
    const auto fromCon = source.find("m_fScale = monster_visual_scale::FromCon(fCon);", body);
    check(body != std::string::npos && summon != std::string::npos && base != std::string::npos &&
        fromCon != std::string::npos && summon < base && base < fromCon,
        "SetCharHeight keeps summons at the base scale before applying CON");
    check(monster_visual_scale::Base < monster_visual_scale::FromCon(5000.0f),
        "the base scale is smaller than a high-CON monster");
    return failures;
}
