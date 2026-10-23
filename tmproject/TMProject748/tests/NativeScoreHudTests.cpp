#include "../internal/ui/NativeScoreHud.h"
#include "../internal/ui/NameColorPolicy748.h"
#include "../internal/ui/SkillRequirementPresentation.h"
#include "../internal/ui/ItemHelpText.h"
#include "../internal/ui/GuildNameDirectory.h"
#include "../internal/ui/CapePresentation748.h"
#include "../internal/application/PremiumMountMagic.h"
#include <cstdio>
#include <cstring>
#include <initializer_list>
#include "SourceMethod.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <memory>
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

    struct MountEffect { short sEffect; short sValue; };
    struct MountDefinition { MountEffect stEffect[12]{}; };
    MountDefinition timedMount{{{EF_NOTRADE, 1}, {EF_DATE, 30}, {EF_MAGIC, 65}}};
    check(premium_mount_magic::Percent(6000, timedMount, 0) == 65,
        "KR magic uses the static exact percentage, not refinement or timer effects");
    check(premium_mount_magic::Percent(3981, timedMount, 55) == 55,
        "premium horses use the mount table instead of static magic");
    MountDefinition namedHorse{{{EF_NOTRADE, 1}}};
    check(premium_mount_magic::Percent(3980, namedHorse, 7) == 7
        && premium_mount_magic::Percent(3990, namedHorse, 45) == 0,
        "only the ten server catalog horses have name-based lifetime");
    for (int index : {0, 2330, 2362, 2389})
        check(premium_mount_magic::Percent(index, timedMount, 0) == 0,
            "empty, egg and adult mounts never receive a second magic multiplier");
    timedMount.stEffect[0].sValue = 0;
    check(premium_mount_magic::Percent(6000, timedMount, 0) == 0,
        "unmarked items do not qualify as timed mounts");
    timedMount.stEffect[0].sValue = 1;
    timedMount.stEffect[1].sValue = 0;
    check(premium_mount_magic::Percent(6000, timedMount, 0) == 0,
        "missing lifetime does not qualify as a timed mount");
    timedMount.stEffect[1] = {EF_HOUR, 24};
    check(premium_mount_magic::Percent(6000, timedMount, 0) == 0,
        "invalid lifetime is rejected just as on the server");
    timedMount.stEffect[1] = {EF_MIN, 1};
    check(premium_mount_magic::Percent(6000, timedMount, 0) == 65,
        "minute-only lifetimes retain the same percentage");
    for (int percent : {7, 35, 45, 55, 65, 72, 145})
        check(premium_mount_magic::Apply(1003, percent) == 1003 * (100 + percent) / 100,
            "magic percentage retains integer rounding without quarter-point truncation");
    check(premium_mount_magic::Apply(1003, 0) == 1003
        && premium_mount_magic::Apply(-1003, 65) == -1003,
        "unequipped mounts and healing remain unchanged");
    check(premium_mount_magic::Apply(2000000000, 72) == (std::numeric_limits<int>::max)(),
        "wide multiplication cannot wrap a projected damage into healing");
    const auto skillDamage = source_contract::Method(ReadNameColorSource("internal/core/BasedefCombat.cpp"),
        "int BASE_GetSkillDamage(int skillnum, STRUCT_MOB* mob, int weather, int weapondamage, int OriginalFace)");
    const auto physicalBranch = skillDamage.find("if (!mob->Class && skind == 1 || mob->Class == 3)");
    const auto magicBranch = skillDamage.find("else\n            {", physicalBranch);
    const auto mountProjection = skillDamage.find("premium_mount_magic::Apply", magicBranch);
    check(physicalBranch != std::string::npos && magicBranch != std::string::npos
        && mountProjection != std::string::npos
        && skillDamage.find("if (skillnum != 97)") < physicalBranch
        && skillDamage.find("if (skillnum == 79)") < physicalBranch
        && skillDamage.find("dam = 5 * (dam *", magicBranch) < mountProjection
        && mountProjection < skillDamage.find("if ((1 << (8 * skillclass))")
        && skillDamage.find("mob->Equip[14].sIndex", magicBranch) < mountProjection,
        "production magic branch reads live slot 14 after amplification, before passives, excluding physical and fixed skills");

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

    struct HelpText { int Color[9]{}; char Help[9][128]{}; };
    HelpText help[2]{};
    const auto helpRecord = [](const std::string& id, const std::string& first) {
        return id + "\n" + first + "\nFFFFFFFF\nFFFFFFFF \nFFFFFFFF \nFFFFFFFF \n"
            "FFFFFFFF \nCC9900FF Native skill description\nCC9900FF Second line\nFFFFFFFF\n";
    };
    std::istringstream helpInput(helpRecord("0", "FFFFFFFF Header")
        + "\n" + helpRecord(" \t1 \t", "CC9900FF " + std::string(200, 'X')));
    check(item_help_748::Load(helpInput, help, 2) == 2, "help reader accepts bare-color blank rows and separators");
    check(static_cast<unsigned int>(help[0].Color[6]) == 0xFFCC9900u &&
        static_cast<unsigned int>(help[0].Color[0]) == 0xFFFFFFFFu,
        "help reader converts the RRGGBBAA prefix to opaque ARGB gold and white");
    check(std::strcmp(help[0].Help[6], "Native skill description") == 0
        && help[0].Help[1][0] == '\0' && help[0].Help[8][0] == '\0',
        "help text preserves native row positions and empty rows");
    check(std::strlen(help[1].Help[0]) == 127 && help[1].Help[0][127] == '\0',
        "long help lines are bounded and terminated without shifting records");
    std::istringstream invalidHelp(helpRecord("-1", "FFFFFFFF Rejected")
        + helpRecord("2", "FFFFFFFF Rejected") + helpRecord("0", "GGFFFFFF Rejected")
        + "1\nFFFFFFFF Partial\n");
    check(item_help_748::Load(invalidHelp, help, 2) == 0
        && std::strcmp(help[0].Help[0], "Header") == 0 && std::strlen(help[1].Help[0]) == 127,
        "invalid IDs, malformed colors and incomplete records do not mutate the table");
    auto runtimeHelp = std::make_unique<HelpText[]>(6500);
    std::istringstream runtimeHelpInput(ReadNameColorSource("../client748/itemhelp.dat"));
    check(item_help_748::Load(runtimeHelpInput, runtimeHelp.get(), 6500) > 300,
        "production itemhelp asset loads complete records");
    item_help_748::LocalizeRepliction(runtimeHelp.get(), 6500);
    for (int index = 4016; index <= 4025; ++index) {
        const auto& record = runtimeHelp[index];
        const char grade = static_cast<char>('A' + (index - 4016) % 5);
        check(std::strstr(record.Help[0], "Repliction") != nullptr
            && record.Help[0][11] == grade, "each Repliction has its own English grade heading");
        check(std::strcmp(record.Help[5], index < 4021
            ? "Maximum refinement: +6." : "Maximum refinement: +9.") == 0,
            "standard and premium Repliction descriptions match server refinement limits");
        for (int row = 0; row < item_help_748::Rows; ++row)
            check(std::strlen(record.Help[row]) <= 42,
                "Repliction descriptions fit one native tooltip line");
    }
    std::istringstream nativeSkillInput(ReadNameColorSource("../client748/skillhelp.dat"));
    check(item_help_748::Load(nativeSkillInput, runtimeHelp.get(), 6500) == 102,
        "all 102 available native skill descriptions load as a separate English corpus");
    for (int book = 5000; book <= 5102; ++book) {
        if (book == 5096) continue; // No prose in the inspected native asset.
        bool hasText = false;
        for (int row = 0; row < item_help_748::Rows; ++row)
            hasText |= runtimeHelp[book].Help[row][0] != '\0';
        check(hasText, "every available native description survives loading, including eighth skills");
    }
    check(std::strcmp(runtimeHelp[5007].Help[2], "Damage: 190 + (Faith/4) x 5 + INT/4.") == 0,
        "Destiny retains native prose and its formula rather than metadata alone");
    check(std::strcmp(runtimeHelp[5085].Help[0], "Opens your cargo storage so you can") == 0 &&
        std::strcmp(runtimeHelp[5085].Help[1], "store items from anywhere.") == 0,
        "Backpack retains the native cargo-storage description");
    // TMFont2 splits any string longer than 42 characters into a second line
    // inside the same row, which overlapped the next tooltip row.
    for (int book = 5000; book <= 5102; ++book)
        for (int row = 0; row < item_help_748::Rows; ++row)
            check(std::strlen(runtimeHelp[book].Help[row]) <= 42,
                "every skill description row fits one native 42-character text line");
    const auto bottomRight = item_help_748::Position(1275, 940, 400, 450, 1280, 960, 1);
    check(bottomRight.x == 880 && bottomRight.y == 480,
        "belt tooltip uses its newly measured height above the cursor and stays on screen");
    const auto topLeft = item_help_748::Position(2, 2, 400, 450, 1280, 960, 1);
    check(topLeft.x == 0 && topLeft.y == 32, "left edge tooltip never leaves the viewport");
    const auto oversized = item_help_748::Position(2, 2, 400, 450, 320, 200, 1);
    check(oversized.x == 0 && oversized.y == 0, "oversized tooltip origin is bounded");
    check(item_help_748::FirstTooltipRow + item_help_748::Rows == 11,
        "nine native help rows fit below the reserved first two tooltip rows");
    const auto mouseOver = source_contract::Method(ReadNameColorSource("internal/ui/SGridTooltip.cpp"),
        "int SGridControl::MouseOver(int nCellX, int nCellY, int bPtInRect)");
    check(mouseOver.find("int nId = item_help_748::FirstTooltipRow") != std::string::npos,
        "production tooltip starts item help at native row two");
    check(mouseOver.find("? pFScene->m_pSkillHelpText[m] : pFScene->m_pParamText[nId]") != std::string::npos,
        "skill prose never shares rows with calculated skill requirements");
    const auto descHeight = source_contract::Method(ReadNameColorSource("internal/ui/SGridTooltip.cpp"),
        "void SGridControl::UpdateDescPanelHeight()");
    check(descHeight.find("float lineStep = pFScene->m_vecDescParamPos[1].y - pFScene->m_vecDescParamPos[0].y;") != std::string::npos &&
        descHeight.find("y += lineStep;") != std::string::npos &&
        descHeight.find("text->m_Font.m_FontSize + 4.0f") == std::string::npos,
        "skill tooltip rows use the native row pitch, not unrendered font metrics");
    check(descHeight.find("text->SetPos(pFScene->m_vecDescParamPos[i].x + offset, y);") <
            descHeight.find("text->SetPos(0, y);"),
        "skill tooltip lists calculated data before the description prose");
    const auto gridInput = source_contract::Method(ReadNameColorSource("internal/ui/SGridInput.cpp"),
        "int SGridControl::OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY)");
    check(gridInput.find("MouseOver(nCellX, nCellY, bPtInRect)") < gridInput.find("UpdateDescPanelHeight()"),
        "current hover is populated before measuring and positioning the tooltip");
    const auto sceneUi = ReadNameColorSource("internal/app/scenes/TMFieldSceneUI.cpp");
    check(sceneUi.find("LoadMsgText(m_pSkillMDesc, (char*)\"UI\\\\descskillm.txt\")") != std::string::npos,
        "skill apprenticeship list loads its existing native explanatory text");
    const auto newApp = ReadNameColorSource("internal/app/scenes/NewApp.cpp");
    check(newApp.find("item_help_748::Load(itemHelp, g_pItemHelp, _countof(g_pItemHelp))") != std::string::npos,
        "production startup uses the tested bounded help reader");
    check(newApp.find("std::ifstream skillHelp(WYD748_TextPath(\"skillhelp.dat\"))") != std::string::npos
        && newApp.find("item_help_748::Load(skillHelp, g_pItemHelp, _countof(g_pItemHelp)) != 102") != std::string::npos,
        "production startup requires the complete native skill-help corpus");

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
