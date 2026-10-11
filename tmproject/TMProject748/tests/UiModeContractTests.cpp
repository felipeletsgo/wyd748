#include "SourceMethod.h"
#include "../internal/core/CameraInput.h"
#include "../internal/core/UiLayout.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <windows.h>

namespace {
struct FakeMouse
{
    HRESULT result = S_OK;
    HRESULT acquireResult = S_OK;
    DIMOUSESTATE2 sample{};
    unsigned int reads = 0;
    unsigned int acquisitions = 0;
    DWORD lastSize = 0;

    HRESULT GetDeviceState(DWORD size, void* output)
    {
        ++reads;
        lastSize = size;
        // Simulate a driver writing data even when the operation fails.
        *static_cast<DIMOUSESTATE2*>(output) = sample;
        return result;
    }
    HRESULT Acquire() { ++acquisitions; return acquireResult; }
};

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

    // Native scene construction uses one 300x16 candidate row at (28, H-20).
    for (const float width : {640.0f, 799.0f, 800.0f, 1024.0f, 1920.0f})
        for (const float height : {480.0f, 600.0f, 768.0f, 1080.0f})
            for (const float scale : {1.0f, 1.25f, 1.8f, 2.0f})
            {
                const auto rect = ImeCandidateLayout(width, height, scale);
                check(rect.x == 28.0f && rect.height == 16.0f &&
                    rect.width == (width < 800.0f ? 350.0f : 300.0f),
                    "the candidate line preserves native geometry and the physical-width threshold");
                check(std::fabs(rect.y * scale - (height - 20.0f * scale)) < 0.001f,
                    "the candidate line stays twenty scaled pixels above the viewport bottom");
            }
    const auto scene = ReadAppSource("internal/app/scenes/TMScene.cpp");
    check(scene.find("const auto candidateRect = ImeCandidateLayout(") != std::string::npos &&
        scene.find("candidateRect.x, candidateRect.y, candidateRect.width, candidateRect.height,") != std::string::npos,
        "scene construction uses the tested IME layout");
    check(scene.find("m_pTextCandidate[0] = new SText") != std::string::npos &&
        scene.find("m_pTextCandidate[nImeCounta] = new SText") == std::string::npos &&
        scene.find("m_pTextCandidate[nImeCount] = 0;") != std::string::npos,
        "only the first candidate control is constructed; spare pointers remain null");
    check(scene.find("m_pTextCandidate[0]->SetVisible(0);") != std::string::npos &&
        scene.find("m_pControlContainer->AddItem(static_cast<SControl*>(m_pTextCandidate[0]));") != std::string::npos,
        "the candidate row starts hidden and belongs to the scene control container");
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
    const auto party = source_contract::Method(ui, "void TMFieldScene::PositionCompatPartyPanel()");
    check(party.find("PartyHintOffsetX(m_pPartyBtn->m_nWidth,") != std::string::npos &&
        party.find("m_pAltText->m_nPosX +") == std::string::npos,
        "reopening the Party panel recomputes the hint from the button width");
    for (const float scale : {0.5f, 1.0f, 1.25f, 1.8f, 4.0f})
    {
        float hintX = 0.0f;
        for (int opening = 0; opening < 100; ++opening)
            hintX = PartyHintOffsetX(20.0f * scale, scale);
        check(hintX == 15.0f * scale, "Party hint offset is stable at every supported scale");
    }

    FakeMouse mouse;
    mouse.sample.lX = 8;
    mouse.sample.lY = -4;
    mouse.sample.lZ = 120;
    mouse.sample.rgbButtons[2] = 0x80;
    DIMOUSESTATE2 sample{};
    check(PollCameraMouse(&mouse, sample) && sample.lX == 8 && sample.lY == -4 &&
        sample.lZ == 120 && sample.rgbButtons[2] == 0x80 && mouse.acquisitions == 0 &&
        mouse.lastSize == sizeof(DIMOUSESTATE2), "successful polling delivers a complete current sample");
    for (const HRESULT failure : {DIERR_INPUTLOST, DIERR_NOTACQUIRED, E_FAIL})
    {
        mouse.result = failure;
        mouse.acquireResult = DIERR_INPUTLOST;
        const auto before = mouse.acquisitions;
        const auto readsBefore = mouse.reads;
        check(!PollCameraMouse(&mouse, sample) && mouse.acquisitions == before + 1 &&
            mouse.reads == readsBefore + 1 && sample.lX == 0 && sample.lY == 0 &&
            sample.lZ == 0 && sample.rgbButtons[2] == 0,
            "failed polling discards deltas and attempts reacquisition only once per frame");
    }
    mouse.result = S_OK;
    mouse.sample = {};
    check(PollCameraMouse(&mouse, sample) && sample.lZ == 0 && sample.rgbButtons[2] == 0,
        "focus recovery does not replay old wheel or rotation input");
    sample.lZ = 120;
    check(!PollCameraMouse<FakeMouse>(nullptr, sample) && sample.lZ == 0,
        "a missing input device produces no sample");
    const auto translator = ReadAppSource("internal/core/EventTranslator.cpp");
    const auto candidates = source_contract::Method(translator, "void EventTranslator::SetVisibleCandidateList(int lParam, int bVisible)");
    check(candidates.find("SText* pCandidateText = pScene->m_pTextCandidate[0];") != std::string::npos &&
        candidates.find("if (pScene->m_pTextCandidate[i] != nullptr)") != std::string::npos &&
        candidates.find("pCandidateText->SetText(candidateText, 0);") != std::string::npos,
        "candidate notifications use the single row and tolerate absent spare controls");
    const auto poll = source_contract::Method(translator, "int EventTranslator::ReadInputEventData()");
    const auto camera = source_contract::Method(translator, "int EventTranslator::CameraEventData()");
    check(poll.find("m_hasMouseSample = PollCameraMouse(m_pMouseDevice, dims2);") != std::string::npos &&
        poll.find("dx = dy = wheel = viewchange = 0;") != std::string::npos,
        "the input translator uses the tested polling path and clears transient deltas");
    check(camera.find("if (!m_hasMouseSample)") < camera.find("TMCamera* pCamera") &&
        camera.find("m_hasMouseSample = false;") < camera.find("TMCamera* pCamera") &&
        camera.find("CameraZoomDistance(") != std::string::npos,
        "camera updates require and consume a fresh sample before accessing camera state");
    for (const int view : {0, 1})
        for (const int rotation : {0, 1})
            for (const int rawWheel : {-120, 120})
            {
                const int wheel = rotation ? -rawWheel : rawWheel;
                const int direction = view == 0 ? rawWheel : wheel;
                check(CameraZoomDistance(5.0f, 1.2f, 10.0f, view, wheel, rawWheel) ==
                    (direction < 0 ? 4.5f : 5.5f),
                    "zoom direction and displacement agree for both camera settings");
                const float atNear = CameraZoomDistance(1.2f, 1.2f, 10.0f, view, wheel, rawWheel);
                const float atFar = CameraZoomDistance(10.0f, 1.2f, 10.0f, view, wheel, rawWheel);
                check(atNear >= 1.2f && atNear <= 1.7f && atFar >= 9.5f && atFar <= 10.0f,
                    "scrolling respects near and far limits and can leave either limit");
            }
    check(CameraZoomDistance(5.0f, 1.2f, 10.0f, 0, -24000, -24000) == 1.2f &&
        CameraZoomDistance(5.0f, 1.2f, 10.0f, 0, 24000, 24000) == 10.0f,
        "multiple wheel notches cannot overshoot the camera bounds");
    check(CameraZoomDistance(5.0f, 1.2f, 10.0f, 0, 0, 0) == 5.0f,
        "no wheel movement preserves the distance");
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
