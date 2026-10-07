#include "SourceMethod.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <windows.h>

namespace {
std::string ReadAppSource(const char* relative)
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

// Native FUN_0055bc0a reads [CLASSIC] but always writes UI mode 2 after closing
// config.txt (0x0055c136). The client must not let CLASSIC=1 select the classic
// atlases again, or the HUD mixes classic and modern icons.
int RunUiModeContractTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool passed, const char* message) {
        ++checks;
        if (!passed) {
            ++failures;
            std::fprintf(stderr, "FAIL ui mode: %s\n", message);
        }
    };

    const auto app = ReadAppSource("internal/app/scenes/NewApp.cpp");
    check(!app.empty(), "NewApp source is available");
    check(app.find("\tg_UIVer = 2;\n") != std::string::npos, "the client always selects UI mode 2");
    check(app.find("g_UIVer = Config.Config[9];") == std::string::npos &&
        app.find("g_UIVer = 1;") == std::string::npos, "config.txt CLASSIC cannot select the classic UI");

    const auto grid = ReadAppSource("internal/ui/SGridItem.cpp");
    check(grid.find("m_GCObj.nTextureSetIndex = g_UIVer == 2 ? 199 : 1;") != std::string::npos,
        "image items take the modern atlas from the single UI mode");

    // FUN_004528c5 mirrors the selected skill damage into Character text 0x450.
    const auto input = ReadAppSource("internal/app/scenes/TMFieldSceneInput.cpp");
    const auto skillKey = source_contract::Method(input, "int TMFieldScene::OnKeyShortSkill(char iCharCode, int lParam)");
    check(skillKey.find("FindControl(TMT_CI_ATRT)") != std::string::npos &&
        skillKey.find("for (SText* target : {m_pSkillDam, pCharSkillDam})") != std::string::npos,
        "skill damage reaches both the HUD cell and Character text 0x450");

    // FieldScene2 has a single EXP progress bar (1171); the later client's ten
    // EXP segments (65655..) are absent, so C.C. must anchor on the native bar.
    const auto automation = ReadAppSource("internal/app/scenes/TMFieldSceneAutomation.cpp");
    check(automation.find("FindControl(TMP_EXP_PROGRESS)") != std::string::npos &&
        automation.find("P_EXP_PROGRESS1") == std::string::npos,
        "the C.C. dialog anchors on the native EXP bar");

    // FUN_004662c5 0x279: System "server" opens panel 0x3000 (list 0x3001)
    // and quits only when that panel is missing; 0x3001 refuses above 699.
    const auto field = ReadAppSource("internal/app/scenes/TMFieldScene.cpp");
    const auto ui = ReadAppSource("internal/app/scenes/TMFieldSceneUI.cpp");
    const auto opener = source_contract::Method(ui, "int TMFieldScene::OpenNativeServerPanel()");
    check(field.find("case 633: // native System: select server\n\t\t{ extractedResult = OpenNativeServerPanel();") != std::string::npos,
        "System button 633 opens the native channel panel");
    check(opener.find("if (!m_pServerPanel)") != std::string::npos &&
        opener.find("m_pServerList->AddItem(server);") != std::string::npos &&
        opener.find("m_pServerPanel->SetVisible(1);") != std::string::npos,
        "the channel panel lists the current group and quits only without a panel");
    check(field.find("if (pItem->m_nCurrent < 700)") != std::string::npos,
        "a channel row is refused only above 699 users");
    check(ui.find("FindControl(TML_MOVE_SELECT_SERVER)") != std::string::npos &&
        ui.find("m_pServerList->SetEventListener(m_pControlContainer);") != std::string::npos,
        "the native channel list 0x3001 is bound and routed to the field scene");

    // Shutdown released the bone animation buffers twice (NewApp::Finalize and
    // ~MeshManager), crashing on exit from the field.
    const auto finalize = source_contract::Method(app, "HRESULT NewApp::Finalize()");
    const auto meshManager = ReadAppSource("internal/render/mesh/MeshManager.cpp");
    const auto release = source_contract::Method(meshManager, "void MeshManager::ReleaseBoneAnimations()");
    check(finalize.find("free(MeshManager::m_BoneAnimationList") == std::string::npos &&
        finalize.find("MeshManager::ReleaseBoneAnimations();") != std::string::npos &&
        release.find("animation = {};") != std::string::npos,
        "bone animation buffers have one idempotent release");

    // FUN_004f59b4 tints ground items with meshes 737..739 by dwColor[mesh - 737];
    // the 7.69 offset 2980 indexed far outside the three colors.
    const auto item = ReadAppSource("internal/game/entities/TMItem.cpp");
    const auto itemRender = source_contract::Method(item, "int TMItem::Render()");
    check(itemRender.find("if (nMeshIndex < 737 || nMeshIndex > 739)") != std::string::npos &&
        itemRender.find("pVertex[i].diffuse = dwColor[nMeshIndex - 737];") != std::string::npos &&
        itemRender.find("nMeshIndex - 2980") == std::string::npos,
        "tinted item meshes index the native color table");

    // FUN_0051a939: an enemy-guild summon is protected outside the castle while
    // no war runs, and a same-cape target always; 7.69 inverted the guild test.
    const auto combat = ReadAppSource("internal/game/entities/TMHumanCombat.cpp");
    const auto moveAttack = source_contract::Method(combat, "void TMHuman::MoveAttack(TMHuman* pTarget)");
    check(moveAttack.find("pTarget->m_cSummons == 1 && g_pObjectManager->m_usWarGuild != pTarget->m_usGuild && pTarget->m_usGuild)") != std::string::npos &&
        moveAttack.find("(!g_bCastleWar && outsideCastle) || (m_cMantua > 0 && pTarget->m_cMantua == m_cMantua)") != std::string::npos,
        "move-attack protects enemy-guild summons like the native filter");

    // FUN_00503a8c chooses the cape placement by skin class (25/26/33/40), so
    // the Titan cape has no lift and the Eden face uses the Beast Master table.
    const auto humanRender = ReadAppSource("internal/game/entities/TMHumanRender.cpp");
    check(humanRender.find("m_sHeadIndex == 2") == std::string::npos &&
        humanRender.find("m_pMantua->Render(0.07f, 1.2f, -0.12f);") != std::string::npos &&
        humanRender.find("m_pMantua->Render(0.01f, 1.0f, 0.0f);") != std::string::npos,
        "cape placement follows the native skin class");
    return failures;
}
