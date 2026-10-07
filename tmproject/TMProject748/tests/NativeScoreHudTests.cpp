#include "../internal/ui/NativeScoreHud.h"
#include "../internal/ui/NameColorPolicy748.h"
#include <cstdio>
#include <cstring>
#include <initializer_list>
#include "SourceMethod.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <windows.h>

namespace {
std::string ReadNameColorSource(const char* relative)
{
    wchar_t executable[MAX_PATH]{};
    const auto length = GetModuleFileNameW(nullptr, executable, MAX_PATH);
    if (!length || length == MAX_PATH)
        return {};
    auto current = std::filesystem::path(executable).parent_path();
    for (int depth = 0; depth < 8 && !current.empty(); ++depth) {
        std::ifstream input(current / "tmproject/TMProject748" / relative, std::ios::binary);
        if (input) {
            std::string source{std::istreambuf_iterator<char>(input), {}};
            source.erase(std::remove(source.begin(), source.end(), '\r'), source.end());
            return source;
        }
        current = current.parent_path();
    }
    return {};
}
}

int RunNativeScoreHudTests(int& checks)
{
    using namespace native_score_hud;
    int failures = 0;
    auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) { ++failures; std::printf("FAIL native score HUD: %s\n", name); }
    };

    const unsigned int palette[] = {0xFF000000u, 0xFFFF0000u, 0xFFFFAA66u,
        0xFFFFFF00u, 0xFFFFFF99u, 0xFF99FF99u, 0xFF99FFFFu, 0xFFFFFFFFu, 0xFFFFF6FFu};
    check(name_color_748::Calculate(1, 75, palette).color == 0xFFFFFF4Cu,
        "signed CP zero retains the native yellow interpolation");
    check(name_color_748::Calculate(999, 140, palette).color == 0xFFE5FFFFu,
        "signed CP +65 is nearly white");
    check(name_color_748::Calculate(1, 145, palette).color == 0xFFFFFFFFu,
        "signed CP +70 reaches the white palette endpoint");
    check(name_color_748::Calculate(1, 10, palette).color == 0xFF3F0000u,
        "signed CP -65 produces dark red");
    for (unsigned int cp = 0; cp < 256; ++cp) {
        const auto player = name_color_748::Calculate(1, cp, palette);
        check(player.border == (cp < 10), "extreme negative CP uses the native black border");
        if (cp < 10)
            check(player.color == 0xFF000000u, "extreme negative CP is black");
        if (cp > 150)
            check(player.color == name_color_748::Calculate(1, 150, palette).color,
                "malformed CP bytes clamp without indexing outside the palette");
        for (unsigned int id : {1000u, 1001u, 65535u}) {
            const auto npc = name_color_748::Calculate(id, cp, palette);
            check(npc.color == 0xFFFFFFFFu && !npc.border, "NPCs are white and never inherit CP");
        }
    }

    const auto createMob = source_contract::Method(ReadNameColorSource(
        "internal/app/scenes/TMFieldSceneWorld.cpp"),
        "int TMFieldScene::OnPacketCreateMobCompat(MSG_STANDARD* pStd)");
    const auto refreshStart = createMob.find("if (pExisting)");
    const auto allocation = createMob.find("auto pHuman = new TMHuman(this);");
    check(refreshStart != std::string::npos && allocation != std::string::npos && refreshStart < allocation,
        "existing entity CP refresh is handled before allocation");
    if (refreshStart != std::string::npos && allocation != std::string::npos && refreshStart < allocation) {
        const auto refresh = createMob.substr(refreshStart, allocation - refreshStart);
        check(refresh.find("pCreateMob->MobName[12]") != std::string::npos &&
            refresh.find("pExisting->UpdateNameColor748();") != std::string::npos &&
            refresh.find("InitObject") == std::string::npos && refresh.find("UpdateScore") == std::string::npos,
            "CP refresh consumes the wire byte without rebuilding skins or resetting movement");
    }
    check(createMob.find("pCreateMob->MobID < 1000 ? 12 : 15") != std::string::npos,
        "player CP metadata is excluded from the displayed name");
    const auto score = source_contract::Method(ReadNameColorSource(
        "internal/game/entities/TMHuman.cpp"), "void TMHuman::UpdateScore(int nGuildLevel)");
    check(score.find("if (m_dwID >= 1000)\n                UpdateNameColor748();") != std::string::npos,
        "caret-prefixed NPC and summon names also receive the white policy");
    const auto labels = ReadNameColorSource("internal/game/entities/TMHumanUI.cpp");
    for (const char* signature : {"void TMHuman::LabelPosition()", "void TMHuman::LabelPosition2()"}) {
        const auto body = source_contract::Method(labels, signature);
        check(body.find("if (m_dwID < 1000 && !m_ucChaosLevel)") != std::string::npos &&
            body.find("if (!m_ucChaosLevel)") == std::string::npos,
            "both name projections restrict the native zero-CP red pulse to players");
    }

    // Carry classification mirrors FUN_004431e4.
    QuickPotionCounts counts;
    AddCarryCell(counts, 4097, 120, 1);  // stacked HP adds its amount
    AddCarryCell(counts, 428, 0, 1);     // stacked HP without EF_AMOUNT adds zero
    AddCarryCell(counts, 685, 5, 1);
    AddCarryCell(counts, 400, 0, 1);     // basic HP counts one per cell
    AddCarryCell(counts, 415, 9, 1);
    AddCarryCell(counts, 3323, 30, 1);   // stacked MP
    AddCarryCell(counts, 691, 2, 1);
    AddCarryCell(counts, 409, 7, 1);     // basic MP counts one per cell
    AddCarryCell(counts, 3378, 4, 230);  // special: amount
    AddCarryCell(counts, 3379, 0, 230);  // special without amount: one
    AddCarryCell(counts, 410, 50, 1);    // unrelated item
    AddCarryCell(counts, 3477, 50, 1);   // not part of the native HP family
    check(counts.hp == 120 + 0 + 5 + 1 + 1, "HP total follows native families");
    check(counts.mp == 30 + 2 + 1, "MP total follows native families");
    check(counts.special == 5, "special total uses amount or one");

    char text[64]{};
    FormatShortcutCount(text, sizeof(text), 17, false);
    check(std::strcmp(text, "17") == 0, "count prints as decimal");
    FormatShortcutCount(text, sizeof(text), 17, true);
    check(std::strcmp(text, "X") == 0, "cancel state prints X");
    FormatExpRange(text, sizeof(text), 606700, 807895);
    check(std::strcmp(text, "606700 / 807895") == 0, "EXP range shares one control");
    FormatExpRange(text, sizeof(text), 5000000000LL, 6000000000LL);
    check(std::strcmp(text, "5000000000 / 6000000000") == 0, "EXP range keeps 64-bit values");

    FormatCritical(text, sizeof(text), 21);
    check(std::strcmp(text, "4.2%") == 0, "critical uses byte << 1 per mille");
    FormatCritical(text, sizeof(text), 0x1FF);
    check(std::strcmp(text, "51.0%") == 0, "critical reads only the native byte");
    FormatMastery(text, sizeof(text), 28, 75);
    check(std::strcmp(text, " 28/75 ") == 0, "mastery cap is left-aligned");
    FormatMastery(text, sizeof(text), 200, 255);
    check(std::strcmp(text, "200/255") == 0, "three-digit mastery fills both fields");
    FormatCharacterName(text, sizeof(text), "FelipeTr");
    check(std::strcmp(text, "[FelipeTr]") == 0, "name is bracketed");
    FormatCharacterName(text, sizeof(text), "ABCDEFGHIJKLMNOPQRS");
    check(std::strcmp(text, "[ABCDEFGHIJKLMNO]") == 0, "name is limited to 15 characters");

    // Hold-to-repeat timing from FUN_004776c3.
    ButtonRepeat repeat;
    check(!repeat.Tick(true, 1000), "first held frame arms the timer");
    check(!repeat.Tick(true, 2500), "no repeat at exactly 1500 ms");
    check(!repeat.Tick(true, 2501), "delay elapsed switches to repeat mode");
    check(!repeat.Tick(true, 2601), "no repeat at exactly 100 ms");
    check(repeat.Tick(true, 2602), "repeat fires after 100 ms");
    check(!repeat.Tick(true, 2650), "repeat waits for the next interval");
    check(repeat.Tick(true, 2703), "repeat continues while held");
    check(!repeat.Tick(false, 2800), "release resets");
    check(!repeat.Tick(true, 2900) && !repeat.Tick(true, 3100), "a new hold restarts the delay");
    repeat.Reset();
    check(repeat.time == 0 && !repeat.repeating, "reset clears the state");
    return failures;
}
