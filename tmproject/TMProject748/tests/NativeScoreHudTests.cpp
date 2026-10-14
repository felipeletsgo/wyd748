#include "../internal/ui/NativeScoreHud.h"
#include "../internal/ui/NameColorPolicy748.h"
#include "../internal/ui/SkillRequirementPresentation.h"
#include "../internal/ui/GuildNameDirectory.h"
#include "../internal/ui/CapePresentation748.h"
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

    for (int characterClass = 0; characterClass < 4; ++characterClass) {
        for (int lineage = 0; lineage < 3; ++lineage) {
            for (int slot = 0; slot < 8; ++slot) {
                const int skill = characterClass * 24 + lineage * 8 + slot;
                check(skill_requirement_748::BookLineage(5000 + skill) == characterClass * 3 + lineage,
                    "standard books retain all twelve native lineage captions");
                check(skill_requirement_748::ManaMastery(skill, characterClass) == lineage + 1,
                    "mana uses the owner's explicit mastery field");
                check(skill_requirement_748::ManaMastery(skill, (characterClass + 1) % 4) == -1,
                    "foreign-class books cannot index outside the mastery array");
                for (int mastery = 1; mastery <= 3; ++mastery)
                    check(skill_requirement_748::RequirementCaption(5000 + skill, mastery)
                        == characterClass * 3 + mastery - 1,
                        "each required mastery uses the book class rather than half its lineage");
            }
            for (int slot = 0; slot < 4; ++slot) {
                const int book = 5400 + characterClass * 12 + lineage * 4 + slot;
                check(skill_requirement_748::BookLineage(book) == characterClass * 3 + lineage,
                    "advanced books retain their native four-entry lineage mapping");
            }
        }
    }
    // Deliberately wide score and unrelated attributes expose the old WORD pointer walk.
    struct TooltipScore { unsigned int Dex = 907, Con = 5; unsigned int Mastery[4] = {0, 0, 10, 200}; };
    const TooltipScore tooltipScore;
    check(skill_requirement_748::RequirementCaption(5046, 3) == 5,
        "Foema Skill Amplify requires Special Magic, never Magic Sword");
    check(skill_requirement_748::MeetsMastery(tooltipScore, 3, 76),
        "200 Special Magic satisfies the reported 76-point requirement");
    check(!skill_requirement_748::MeetsMastery(tooltipScore, 2, 76),
        "10 Black Magic does not satisfy 76 points despite high intelligence");
    check(skill_requirement_748::MeetsMastery(tooltipScore, 3, 200)
        && !skill_requirement_748::MeetsMastery(tooltipScore, 3, 201), "mastery comparison includes equality");
    for (int invalid : {-1, 4999, 5096, 5104, 5399, 5448, 65535})
        check(skill_requirement_748::BookLineage(invalid) == -1
            && skill_requirement_748::RequirementCaption(invalid, 3) == -1,
            "non-lineage books never index the twelve-entry caption table");
    check(!skill_requirement_748::MeetsMastery(tooltipScore, 0, 1)
        && !skill_requirement_748::MeetsMastery(tooltipScore, 4, 1)
        && !skill_requirement_748::MeetsMastery(tooltipScore, 3, -1), "invalid mastery requirements are rejected");
    const auto tooltip = source_contract::Method(ReadNameColorSource("internal/ui/SGridTooltip.cpp"),
        "void SGridControl::DescribeSkillItem(SGridControlItem*& pItem, TMFieldScene*& pFScene, unsigned int& dwColor, int& nLineId, char (&szDesc)[128])");
    check(tooltip.find("skill_requirement_748::RequirementCaption") != std::string::npos
        && tooltip.find("skill_requirement_748::MeetsMastery") != std::string::npos
        && tooltip.find("skill_requirement_748::ManaMastery") != std::string::npos
        && tooltip.find("unsigned short*") == std::string::npos,
        "production skill tooltip consumes the typed presentation helpers");

    guild_name_748::Directory guildNames;
    std::istringstream guildFile("0 0 1 ADMIN\n0 1 1 OTHER_CHANNEL\n1 0 1 OTHER_GROUP\n"
        "malformed\n-1 0 1 INVALID\n0 16 1 INVALID\n0 0 4096 INVALID\n"
        "0 0 0 INVALID\n0 0 2 ABCDEFGHIJKLM\n0 0 3 TEAM_name alias1 alias2 alias3\n");
    guildNames.Load(guildFile);
    check(guildNames.Find(0, 1) == "ADMIN", "native guild file resolves ADMIN");
    check(guildNames.Find(0, 4097) == "OTHER_CHANN", "guild channel is decoded from its high nibble");
    check(guildNames.Find(1, 1) == "OTHER_GROUP", "server groups never share guild labels");
    check(guildNames.Find(0, 2) == "ABCDEFGHIJK", "guild names fit the native eleven-character payload");
    check(guildNames.Find(0, 3) == "TEAM_name", "optional native aliases do not become part of the guild name");
    check(guildNames.Find(0, 0).empty() && guildNames.Find(0, 4).empty()
        && guildNames.Find(-1, 1).empty() && guildNames.Find(10, 1).empty()
        && guildNames.Find(0, 65536).empty(), "missing and invalid guild identities stay empty");
    std::istringstream emptyGuildFile;
    guildNames.Load(emptyGuildFile);
    check(guildNames.Find(0, 1).empty(), "reload removes stale names after guild deletion");
    const auto hud = source_contract::Method(ReadNameColorSource("internal/app/scenes/TMFieldSceneUI.cpp"),
        "void TMFieldScene::UpdateCompatScoreUI()");
    check(hud.find("guildNames.Refresh(\"Guilds.txt\")") != std::string::npos
        && hud.find("guildNames.Find(g_pObjectManager->m_nServerGroupIndex, pMobData->Guild)") != std::string::npos,
        "Character Guild resolves the authoritative identity using the launcher file");

    const auto selectInput = source_contract::Method(ReadNameColorSource(
        "internal/app/scenes/TMSelectCharSceneInput.cpp"),
        "int TMSelectCharScene::OnMouseEventWhileSelecting(TMHuman*& pOver)");
    check(selectInput.find("m_stSelCharData.Guild[i]") != std::string::npos
        && selectInput.find("guildNames.Refresh(\"Guilds.txt\")") != std::string::npos
        && selectInput.find("stEffect[1].cEffect == 28") == std::string::npos,
        "selection resolves the packet guild ID rather than a face-effect byte");
    for (int refinement = 0; refinement <= 15; ++refinement) {
        check(cape_presentation_748::Refinement(refinement) == refinement,
            "cape material preserves every supported refinement tier");
        for (int cape : {545, 546, 734, 4095})
            check(cape_presentation_748::PackedRefinement(
                static_cast<unsigned short>(cape | (refinement << 12))) == refinement,
                "compact cape refinement is independent of the item index");
    }
    check(cape_presentation_748::Refinement(-1) == 0
        && cape_presentation_748::Refinement(16) == 15,
        "full cape refinement cannot address past the material table");
    const auto appearance = ReadNameColorSource("internal/game/entities/TMHumanAppearance.cpp");
    const auto fullCape = source_contract::Method(appearance,
        "void TMHuman::SetPacketMOBItem(STRUCT_MOB* pMobData)");
    const auto packedCape = source_contract::Method(appearance,
        "void TMHuman::SetPacketEquipItem(unsigned short* sEquip)");
    check(fullCape.find("BASE_GetItemSanc(&pMobData->Equip[15])") != std::string::npos
        && fullCape.find("cape_presentation_748::Refinement") != std::string::npos
        && fullCape.find("m_nTotalKill") == std::string::npos,
        "full equipment derives cape glow from the authoritative item, never kills");
    check(packedCape.find("cape_presentation_748::PackedRefinement(sEquip[15])") != std::string::npos
        && packedCape.find("m_nTotalKill") == std::string::npos,
        "visible equipment updates derive cape glow from the refinement nibble");
    for (const auto* method : {&fullCape, &packedCape})
        check(method->find("m_ucMantuaSanc = 0;") != std::string::npos,
            "unequipping the cape clears its previous refinement");

    constexpr int capeBases[] = {19, 3, 2, 6, 1, 0, 7, 25, 24, 26, 28, 27, 29, 32, 31, 33};
    for (int branch = 0; branch < 16; ++branch) {
        for (int channel = 0; channel < 10; ++channel) {
            const int skin = 40 + branch * 10 + channel;
            check(cape_presentation_748::BaseSkin(skin) == capeBases[branch],
                "every citizen cape decodes to the original base texture");
            const int expected = capeBases[branch] == 19 ? -1 : capeBases[branch];
            check(cape_presentation_748::KingdomEmblem(skin, 0) == expected,
                "all citizen cape emblems fit the native UI set or remain neutral");
            for (int helmet = 3503; helmet <= 3506; ++helmet)
                check(cape_presentation_748::KingdomEmblem(skin, helmet)
                    == (capeBases[branch] == 2 ? 33 : expected),
                    "kingdom helmet override runs after citizenship normalization");
        }
    }
    for (int base = 0; base <= 36; ++base)
        check(cape_presentation_748::KingdomEmblem(base, 0) == (base == 19 ? -1 : base),
            "non-citizen capes preserve the native emblem index");
    for (int invalid : {-1, 37, 39, 200, 201, 65535})
        check(cape_presentation_748::KingdomEmblem(invalid, 0) == -1,
            "invalid cape variants cannot address past the UI texture set");
    for (const auto* method : {&hud, &selectInput})
        check(method->find("cape_presentation_748::KingdomEmblem") != std::string::npos
            && method->find("FindControl(TMT_NOKINGDOM)") != std::string::npos
            && method->find("pNoKingdom->SetVisible(kingdomEmblem < 0)") != std::string::npos
            && method->find("m_pMantua->m_Look.Skin0") == std::string::npos,
            "both screens project the cape state independently of mesh allocation and hide None");

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

    FormatCritical(text, sizeof(text), 96);
    check(std::strcmp(text, "9.6%") == 0, "critical shares the item tooltip per-mille unit");
    FormatCritical(text, sizeof(text), 1000);
    check(std::strcmp(text, "100.0%") == 0, "critical reaches 100 percent");
    FormatCritical(text, sizeof(text), 1500);
    check(std::strcmp(text, "100.0%") == 0, "critical display is capped at 100 percent");
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
