#include "SourceMethod.h"
#include "../internal/core/CameraInput.h"
#include "../internal/core/UiLayout.h"
#include "../internal/application/NativeSpecialSkills.h"
#include "../internal/application/NativeCostumeSlot.h"
#include "../internal/application/InstanceTimerProjection.h"

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

    // FUN_004431e4 (UI mode 2): special bits 24..31 become items 5096..5103 in
    // the 4x2 grid 571. Dropping this loop made Sephira books learn invisibly.
    for (int i = 0; i < native_special_skill::SlotCount; ++i)
    {
        const auto cell = native_special_skill::SlotCell(i);
        check(native_special_skill::Bit(i) == (1u << (24 + i)) &&
            native_special_skill::ItemIndex(i) == 5096 + i &&
            cell.x == i % 4 && cell.y == i / 4,
            "special skill slots follow the native modern-UI projection");
    }
    check(native_special_skill::ItemIndex(1) == 5000 + 97 && native_special_skill::Bit(1) == 0x02000000u &&
        native_special_skill::ItemIndex(5) == 5000 + 101 && native_special_skill::Bit(5) == 0x20000000u,
        "Conjure Cannon and Spectral Force map to the server's learned bits 25 and 29");
    const auto fieldUi = ReadAppSource("internal/app/scenes/TMFieldSceneUI.cpp");
    const auto learnedUi = source_contract::Method(fieldUi, "void TMFieldScene::UpdateCompatLearnedSkillUI()");
    check(learnedUi.find("native_special_skill::SlotCount") != std::string::npos &&
        learnedUi.find("m_pGridSkillBelt->AddItem(pControlItem, cell.x, cell.y)") != std::string::npos &&
        learnedUi.find("m_pGridSkillBelt->PickupItem(cell.x, cell.y)") != std::string::npos,
        "the learned-skill projection fills and clears the special skill grid");
    // FUN_00435b13 types only belts 573/586 as grid 5; grid 571 is a skill list
    // whose items are picked up and dropped on a belt like class skills.
    const auto skillsSource = ReadAppSource("internal/app/scenes/TMFieldSceneSkills.cpp");
    const auto beltInit = source_contract::Method(skillsSource, "void TMFieldScene::InitializeCompatSkillBelts()");
    check(beltInit.find("m_pGridSkillBelt->m_eGridType") == std::string::npos &&
        beltInit.find("m_pGridSkillBelt2->m_eGridType = TMEGRIDTYPE::GRID_SKILLB;") != std::string::npos &&
        beltInit.find("m_pGridSkillBelt3->m_eGridType = TMEGRIDTYPE::GRID_SKILLB;") != std::string::npos,
        "the special skill list is not a shortcut belt");
    const auto shortcutKeys = ReadAppSource("internal/app/scenes/TMFieldSceneInput.cpp");
    check(shortcutKeys.find("SetShortSkill(CodeIndex + nBase, pItem);") != std::string::npos &&
        shortcutKeys.find("m_pGridSkillBelt->m_nColumnGridCount") != std::string::npos &&
        shortcutKeys.find("SetShortSkill(nBase, pItem);") == std::string::npos,
        "hovering a special skill binds it to the pressed key through the grid layout");

    // FUN_00486808/FUN_004a626e: Equip[13] feeds the familiar field. Zeroing it
    // as a costume-only slot hid equipped Imp, Naiads and Angel familiars.
    const auto effects = ReadAppSource("internal/game/entities/TMHumanEffects.cpp");
    const auto setFamiliar = source_contract::Method(effects, "void TMHuman::SetFamiliarItem(short itemIndex)");
    check(setFamiliar.find("if (m_sFamiliar != itemIndex && m_pFamiliar)") != std::string::npos &&
        setFamiliar.find("m_pFamiliar = nullptr;") != std::string::npos &&
        setFamiliar.find("m_sFamiliar = itemIndex;") != std::string::npos,
        "a changed familiar disposes the previous pet before the new item is published");
    const auto frameEffect = source_contract::Method(effects, "void TMHuman::FrameMoveEffect(unsigned int dwServerTime)");
    check(frameEffect.find("g_pObjectManager->DeleteObject(m_pFamiliar);\n        m_pFamiliar = nullptr;") != std::string::npos,
        "a shadowed human keeps no pointer to its released familiar");
    // Native FUN_00506f9d: persistent buff parameters are independent of
    // character scale. These source contracts do not prove pixel parity.
    const auto buffCode = [](const std::string& body) {
        auto code = source_contract::MaskCpp(body);
        code.erase(std::remove_if(code.begin(), code.end(), [](unsigned char c) {
            return std::isspace(c) != 0;
        }), code.end());
        return code;
    };
    const auto skillAmpContract = [&](const std::string& body) {
        const auto code = buffCode(body);
        return code.find("kSkillAmpGroundOffset=0.4f;") != std::string::npos &&
            code.find("newTMEffectBillBoard2(93,0,1.5f,1.5f,1.5f,0.0f,5000)") != std::string::npos &&
            code.find("m_pSkillAmp->m_nFade=3;") != std::string::npos &&
            code.find("m_pSkillAmp->SetColor(0x88888800);") != std::string::npos;
    };
    check(skillAmpContract(frameEffect), "Skill Amp preserves the native plane scale, height, fade and color");
    auto reducedAura = frameEffect;
    const auto nativeAura = reducedAura.find("93, 0, 1.5f, 1.5f, 1.5f");
    if (nativeAura != std::string::npos)
        reducedAura.replace(nativeAura, std::string("93, 0, 1.5f, 1.5f, 1.5f").size(), "93, 0, 0.5f, 0.5f, 0.5f");
    check(nativeAura != std::string::npos && !skillAmpContract(reducedAura),
        "the Skill Amp contract rejects the one-third-size regression");
    const auto amp = buffCode(source_contract::Method(frameEffect, "if (m_cSKillAmp == 1)"));
    const auto firstHeight = amp.find("m_fHeight+kSkillAmpGroundOffset");
    check(firstHeight != std::string::npos &&
        amp.find("m_fHeight+kSkillAmpGroundOffset", firstHeight + 1) != std::string::npos &&
        amp.find("m_vecSkinPos") == std::string::npos,
        "Skill Amp creation and subsequent frames both follow terrain, including mounts");
    const auto ampRelease = buffCode(source_contract::Method(frameEffect, "else if (m_pSkillAmp)"));
    check(ampRelease.find("DeleteObject(m_pSkillAmp)") < ampRelease.find("m_pSkillAmp=nullptr;"),
        "removing Skill Amp releases its persistent plane before clearing the pointer");
    const auto hidden = buffCode(source_contract::Method(effects, "ExtractedFlow TMHuman::UpdateHiddenEffects()"));
    check(hidden.find("m_pSkillAmp->m_bShow=0;") != std::string::npos,
        "hidden humans do not leak the restored Skill Amp aura");
    const auto code = buffCode(frameEffect);
    check(code.find("floatfScale=1.0f;") != std::string::npos &&
        code.find("fScale=1.3f;") != std::string::npos,
        "Lighten retains native foot and mounted scales");
    check(code.find("dwServerTime%1000)/1000.0f") != std::string::npos &&
        code.find("dwServerTime-m_dwElimental>500") != std::string::npos,
        "Immunity rotation and Elemental emission use native periods");
    const auto mana = buffCode(source_contract::Method(frameEffect,
        "if (!g_bHideSkillBuffEffect && m_cManaControl == 1)"));
    check(mana.find("newTMEffectBillBoard(56,1500,") != std::string::npos &&
        mana.find("newTMEffectBillBoard(0,1500,") != std::string::npos &&
        mana.find("AddChild(mpBill);") != std::string::npos &&
        mana.find("AddChild(mpBill2);") != std::string::npos &&
        mana.find("if(mpBill)") != std::string::npos && mana.find("if(mpBill2)") != std::string::npos,
        "visible Mana Control emits both owned finite-lived particles with allocation guards");
    check(mana.find("m_fParticleV=-1.2f;") != std::string::npos &&
        mana.find("m_fParticleV=1.5f;") != std::string::npos &&
        mana.find("*0.07f+m_vecPosition.y") != std::string::npos &&
        mana.find("*0.1f+m_vecPosition.y") != std::string::npos,
        "Mana Control retains native particle directions and spread");
    for (const char axis : {'x', 'y', 'z'})
        check(code.find(std::string("m_pProtector->m_pSkinMesh->m_vScale.") + axis + "=0.5f;") != std::string::npos,
            "Protector retains native scale on each axis");
    const auto critical = buffCode(source_contract::Method(frameEffect,
        "if (!g_bHideSkillBuffEffect && m_cCriticalArmor == 1)"));
    check(critical.find("m_fScaleH=0.7f;") != std::string::npos &&
        critical.find("m_fScaleV=0.7f;") != std::string::npos &&
        critical.find("m_vecSkinPos.y)-0.3f") != std::string::npos &&
        critical.find("m_fHeight)+0.3f") != std::string::npos &&
        critical.find("m_fHeight),m_vecPosition.y") != std::string::npos,
        "Critical Armor preserves native nonzero scales and creation/follow heights");
    check(code.find("m_pSoul[i]->m_fScale=0.4f;") != std::string::npos &&
        code.find("/2.0f+m_vecSkinPos.y") != std::string::npos &&
        code.find("/2.0f+m_fHeight") != std::string::npos,
        "Soul preserves native mesh scale and initial half-height anchors");
    check(code.find("newTMEffectBillBoard2(94,0,1.5f,1.5f,1.5f,0.0,3000)") != std::string::npos &&
        code.find("newTMEffectBillBoard2(94,0,1.2f,1.2f,1.2f,0.0f,5000)") != std::string::npos,
        "the neighboring Aurora and Shadow planes keep their native scales and cycles");
    const auto slowSlash = buffCode(source_contract::Method(humanRender, "else if(m_cSlowSlash == 1)"));
    check(slowSlash.find("color.r=fR/2.0f;") != std::string::npos &&
        slowSlash.find("m_materials.Specular=color;") < slowSlash.find("color.r=fR;") &&
        slowSlash.find("color.r=fR;") < slowSlash.find("m_materials.Diffuse=color;"),
        "Slow Slash retains half-strength emission and full-strength diffuse red");
    const auto swings = ReadAppSource("internal/render/effects/core/TMEffectSWSwing.cpp");
    const auto magic = buffCode(source_contract::Method(swings,
        "if (m_cMagicWeapon == 1 && !g_bHideEffect && dwServerTime - m_dwOldTime > 50)"));
    check(magic.find("j<3;") != std::string::npos && magic.find("rand()%5") != std::string::npos &&
        magic.find("newTMEffectBillBoard(56,500,") != std::string::npos &&
        magic.find("newTMEffectBillBoard(60,500,") != std::string::npos,
        "Magic Weapon emits three native pairs with bounded randomness and finite lifetimes");
    check(magic.find("0.2f-(float)j*0.01f,0.2f-(float)j*0.01f,0.2f-(float)j*0.01f,0.002f,1,80") != std::string::npos &&
        magic.find("fRand*0.01f+0.2f,fRand*0.01f+0.2f,fRand*0.01f+0.2f,0.0005f,1,80") != std::string::npos,
        "both Magic Weapon particle families retain native dimensions and velocities");
    check(magic.find("SetColor(dwColor[j]);") != std::string::npos &&
        magic.find("dwColor[0]=0xFF550088;") != std::string::npos &&
        magic.find("dwColor[1]=0xFF555500;") != std::string::npos &&
        magic.find("dwColor[2]=0xFF005500;") != std::string::npos &&
        magic.find("SetColor(0xFF333355);") != std::string::npos &&
        magic.find("m_fAxisAngle=(float)(3.1415927f*fRand)/3.0f;") != std::string::npos,
        "Magic Weapon retains its native colored cores and secondary rotation");
    check(magic.find("if(pChild)") != std::string::npos && magic.find("if(pChild2)") != std::string::npos &&
        magic.find("AddChild(pChild);") != std::string::npos && magic.find("AddChild(pChild2);") != std::string::npos &&
        magic.find("vecStartOut.y+0.2f") != std::string::npos && magic.find("m_dwOldTime=dwServerTime;") != std::string::npos,
        "weapon emissions keep allocation guards, scene ownership, height and clock advancement");
    const auto armor = buffCode(source_contract::Method(swings,
        "if (m_cArmorClass == 1 && !g_bHideEffect && dwServerTime - m_dwOldTime > 100)"));
    check(armor.find("(float)(fRand*0.01f)+0.1f,(float)(fRand*0.01f)+0.1f,(float)(fRand*0.01f)+0.1f") != std::string::npos &&
        armor.find("0.009f") == std::string::npos,
        "Armor Class secondary particles retain native spread");
    const auto enchant = buffCode(source_contract::Method(swings, "if (m_bEnchant)"));
    check(enchant.find("0.0001f,1,80") != std::string::npos &&
        enchant.find("newTMEffectBillBoard(56,0,1.0f,1.0f,1.0f,0.0f,1,80)") != std::string::npos,
        "Enchant retains native particle velocity and persistent plane size");
    const auto appearance = ReadAppSource("internal/game/entities/TMHumanAppearance.cpp");
    const auto inventoryPackets = ReadAppSource("internal/app/scenes/TMFieldSceneInventoryPackets.cpp");
    const auto selectChar = ReadAppSource("internal/app/scenes/TMSelectCharScene.cpp");
    check(appearance.find("SetFamiliarItem(pMobData->Equip[costume_slot::FamiliarSlot].sIndex);") != std::string::npos &&
        appearance.find("SetFamiliarItem(static_cast<short>(sEquip[costume_slot::FamiliarSlot]));") != std::string::npos &&
        inventoryPackets.find("m_pMyHuman->SetFamiliarItem(pMobData->Equip[costume_slot::FamiliarSlot].sIndex);") != std::string::npos &&
        selectChar.find("m_pHuman[i]->SetFamiliarItem(pSelChar->Equip[i][costume_slot::FamiliarSlot].sIndex);") != std::string::npos,
        "self, other players and character selection project slot 13 as the familiar");

    // WYD-Go costume-slot extension (server model/costume_slot.go): mesh-36
    // slot-13 costumes equip in slot 12, so a costume and a familiar coexist.
    static_assert(costume_slot::CostumeSlot == 12 && costume_slot::FamiliarSlot == 13 &&
        costume_slot::CostumeSlotMask == 4096 && costume_slot::NativeCostumeSlotMask == 8192);
    check(costume_slot::RemapPosition(36, 8192) == 4096 && costume_slot::RemapPosition(36, 4096) == 4096 &&
        costume_slot::RemapPosition(46, 8192) == 8192 && costume_slot::RemapPosition(2773, 8192) == 8192 &&
        costume_slot::RemapPosition(36, 4) == 4 && costume_slot::RemapPosition(10, 4096) == 4096,
        "only native slot-13 costumes move to the costume slot");
    check(costume_slot::IsCostume(36, 4096) && costume_slot::IsCostume(36, 8192) &&
        !costume_slot::IsCostume(10, 4096) && !costume_slot::IsCostume(36, 4),
        "a guild medal in slot 12 is not a costume");
    const auto assets = ReadAppSource("internal/core/WYD748Assets.cpp");
    check(assets.find("destination.nPos = costume_slot::RemapPosition(source.IndexMesh, source.Position);") != std::string::npos,
        "ItemList.bin positions follow the server costume-slot catalog");
    check(appearance.find("m_sCostume = pMobData->Equip[costume_slot::CostumeSlot].sIndex;") != std::string::npos &&
        appearance.find("const int nCostumeIndex = sEquip[costume_slot::CostumeSlot];") != std::string::npos &&
        inventoryPackets.find("m_pMyHuman->m_sCostume = pMobData->Equip[costume_slot::CostumeSlot].sIndex;") != std::string::npos &&
        selectChar.find("m_pHuman[i]->m_sCostume = pSelChar->Equip[i][costume_slot::CostumeSlot].sIndex;") != std::string::npos,
        "self, other players and character selection draw the costume from slot 12");
    const auto fieldInput = ReadAppSource("internal/app/scenes/TMFieldSceneInput.cpp");
    const auto autoKey = source_contract::Method(fieldInput, "int TMFieldScene::OnKeyAuto(char iCharCode, int lParam)");
    check(autoKey.find("costume_slot::IsCostume(") != std::string::npos,
        "the medal-gated F action ignores a costume in slot 12");

    // FUN_004662c5: the Inventory X (0x170) runs the FUN_00447691 cascade, so a
    // Cargo opened with the Inventory closes with it.
    const auto fieldScene = ReadAppSource("internal/app/scenes/TMFieldScene.cpp");
    const auto closeStart = fieldScene.find("case 368: // native inventory/shop close button");
    const auto closeEnd = fieldScene.find("case 533:", closeStart);
    const auto closeBody = closeStart != std::string::npos && closeEnd != std::string::npos
        ? fieldScene.substr(closeStart, closeEnd - closeStart) : std::string{};
    check(closeBody.find("if (m_pInvenPanel && m_pInvenPanel->IsVisible())\n\t\t\tSetVisibleInventory();") != std::string::npos &&
        closeBody.find("m_pInvenPanel->SetVisible(0)") == std::string::npos,
        "the Inventory X closes Cargo and the other peers through the native cascade");
    // Server ExpiryDisplayEffects projects the costume expiry date; every
    // costume, including imported KR rows above 4420, must show it.
    const auto tooltip = ReadAppSource("internal/ui/SGridTooltip.cpp");
    check(tooltip.find("costume_slot::IsCostume(g_pItemList[pItem->m_pItem->sIndex].nIndexMesh,") != std::string::npos,
        "the costume tooltip shows the projected expiry for every costume");
    // Mounted speed is fixed; EF_RUNSPEED on a mount row is the server's final
    // speed, never a "Moving Speed Increase" bonus line.
    check(tooltip.find("if (dwEFParam[l] == EF_RUNSPEED && (g_pItemList[pItem->m_pItem->sIndex].nPos & 0x4000))\n\t\t\tcontinue;") != std::string::npos,
        "the mount tooltip omits the run-speed bonus line");
    // The native minimap control stores -45 degrees to match the default
    // camera, the arrow and TMHuman markers. The renderer rotates the texture
    // inside the axis-aligned panel instead of turning the crop into a diamond.
    const auto minimapUi = ReadAppSource("internal/app/scenes/TMFieldSceneUI.cpp");
    const auto primitives = ReadAppSource("internal/platform/windows/RenderDevicePrimitives.cpp");
    check(minimapUi.find("RenderDevice::m_pTextureRotationControl = m_pMiniMapPanel->GetGeomControl();\n\t\t\tRenderDevice::m_fTextureRotation = -0.78539819f;") != std::string::npos &&
        minimapUi.find("m_pMiniMapPanel->GetGeomControl()->fAngle = 0.0f;") != std::string::npos &&
        primitives.find("ipControl == m_pTextureRotationControl") != std::string::npos &&
        primitives.find("const float fSin = sinf(-m_fActiveTextureRotation);") != std::string::npos,
        "the minimap texture uses the native -45 degree orientation inside its frame");
    // WYD.exe 0x47a336/FUN_0044ca65: the UI2 header above the compact map
    // shows "<server group>-<channel>" and is hidden while expanded.
    const auto minimapWorld = ReadAppSource("internal/app/scenes/TMFieldSceneWorld.cpp");
    const auto minimapFrame = ReadAppSource("internal/app/scenes/TMFieldScene.cpp");
    check(minimapWorld.find("m_pMiniMapServerPanel->SetPos(0, px(-19.0f));") != std::string::npos &&
        minimapWorld.find("m_pMiniMapServerPanel->SetVisible(ui2 && layout.visible && !layout.expanded);") != std::string::npos &&
        minimapFrame.find("sprintf_s(server, \"%s-%d\", g_szServerNameList[group], g_pObjectManager->m_nServerIndex);") != std::string::npos,
        "the compact minimap header shows the native server and channel label");
    // FPS/ping row under the minimap coordinates; ping is the socket's TCP RTT.
    const auto socketSource = ReadAppSource("internal/platform/windows/CPSock.cpp");
    check(minimapFrame.find("sprintf_s(stats, \"FPS: %d  Ping: %d ms\"") != std::string::npos &&
        minimapWorld.find("m_pNetStatsText->SetPos(0, mapSize + px(coordinatesInside ? 2.0f : 20.0f));") != std::string::npos &&
        socketSource.find("SIO_TCP_INFO") != std::string::npos &&
        minimapFrame.find("if (!_stricmp(str1, \"fps\"))") != std::string::npos &&
        minimapWorld.find("m_pNetStatsText->SetVisible(layout.visible && m_bShowNetStats);") != std::string::npos,
        "the local /fps command toggles FPS and TCP round-trip time below the minimap coordinates");
    // MeshManager never fills m_sAnimationArray[3]; the BM Wolf/Bear forms
    // must read AniSound4 file indices directly or every motion is frame 0.
    const auto humanAnimation = ReadAppSource("internal/game/entities/TMHumanAnimation.cpp");
    const auto riderMeshAnimations = ReadAppSource("internal/render/mesh/MeshManager.cpp");
    check(humanAnimation.find("else if (m_nSkinMeshType == 3)\n                {\n                    if (!m_pSkinMesh->SetAnimation(g_MobAniTable[m_nSkinMeshType].dwAniTable[(int)eMotion])") != std::string::npos &&
        riderMeshAnimations.find("MeshManager::m_sAnimationArray[3][") == std::string::npos,
        "the wolf/bear transformation animates from its own animation table");
    // A gate received before its region's warp must survive the height-window
    // rebuild; otherwise the client walks through a gate the server enforces.
    const auto sceneGround = ReadAppSource("internal/app/scenes/TMSceneGround.cpp");
    const auto worldScene = ReadAppSource("internal/app/scenes/TMFieldSceneWorld.cpp");
    const auto inventoryScene = ReadAppSource("internal/app/scenes/TMFieldSceneInventoryPackets.cpp");
    std::size_t reapplied = 0;
    for (auto at = sceneGround.find("memcpy(m_GateMapData"); at != std::string::npos; at = sceneGround.find("memcpy(m_GateMapData", at + 1))
        if (sceneGround.find("RefreshGroundObjects();", at) < sceneGround.find("SaveHeightMap(", at))
            ++reapplied;
    check(reapplied == 2 &&
        worldScene.find("pItem->m_nMaskHeight = static_cast<unsigned char>(pMsg->Height);") != std::string::npos &&
        inventoryScene.find("pItem->m_nMaskHeight = static_cast<unsigned char>(pMsg->Height);") != std::string::npos,
        "both height rebuilds re-seat ground objects and re-stamp known gates");
    // Mesh 36 remains the costume identity but resolves to sword16.msa; grid
    // and ground items present costumes with the KR costume mesh instead.
    static_assert(costume_slot::IconMesh == 2885 && costume_slot::IconMesh != costume_slot::CostumeMesh);
    const auto meshIndex = source_contract::Method(ReadAppSource("internal/core/BasedefAppearance.cpp"),
        "int BASE_GetMeshIndex(short sIndex)");
    check(meshIndex.find("if (costume_slot::IsCostume(g_pItemList[sIndex].nIndexMesh, g_pItemList[sIndex].nPos))\n        return costume_slot::IconMesh;") != std::string::npos,
        "costume grid and ground icons use the costume mesh, not the sword");
    const auto timedTooltip = source_contract::Method(tooltip,
        "void SGridControl::DescribeTimedItem(SGridControlItem*& pItem, TMFieldScene*& pFScene, int& nLineId, char (&szDesc)[128])");
    check(timedTooltip.find("sprintf(szDesc, \"%s %s %s\", formattedDate, formattedHour, formattedMin);") != std::string::npos &&
        timedTooltip.find("g_pMessageStringTable[298]") == std::string::npos,
        "the familiar tooltip shows only the remaining day/hour/minute");
    // 0x3A1 starts the green countdown for instances and quest zones; the
    // field FrameMove must refresh it every frame.
    check(instance_timer::Remaining(600, 1000, 1000) == 600 &&
        instance_timer::Remaining(600, 1000, 1999) == 600 &&
        instance_timer::Remaining(600, 1000, 2000) == 599 &&
        instance_timer::Remaining(600, 1000, 600000) == 1 &&
        instance_timer::Remaining(600, 1000, 601000) == 0 &&
        instance_timer::Remaining(600, 1000, 602000) == 0,
        "the countdown expires at the server duration without rounding early");
    check(instance_timer::Remaining(0, 0, 0) == 0 &&
        instance_timer::Remaining(-1, 0, 0) == 0 &&
        instance_timer::Remaining(600, 0xFFFFFF00u, 0x000002E8u) == 599 &&
        instance_timer::Remaining(60, 5000, 5000) == 60,
        "stop, invalid duration, tick wrap and a replaced countdown are safe");
    const auto events = ReadAppSource("internal/app/scenes/TMFieldSceneEvents.cpp");
    const auto timerHud = source_contract::Method(events, "void TMFieldScene::UpdateInstanceTimerHud(unsigned int dwServerTime)");
    check(timerHud.find("instance_timer::Remaining(m_nLastTime, m_dwStartRankTime, dwServerTime)") != std::string::npos &&
        timerHud.find("sprintf_s(szTime, \"%02d : %02d\", remaining / 60, remaining % 60);") != std::string::npos &&
        timerHud.find("m_pRankTimeText->SetVisible(0);") != std::string::npos &&
        fieldScene.find("UpdateInstanceTimerHud(dwServerTime);") != std::string::npos,
        "the field scene draws and expires the instance/quest countdown");
    const auto inventory = ReadAppSource("internal/app/scenes/TMFieldSceneInventory.cpp");
    const auto cascade = source_contract::Method(inventory, "void TMFieldScene::SetVisibleInventory()");
    check(cascade.find("m_pCargoPanel, m_pShopPanel, m_pHellgateStore, m_pInputGoldPanel") != std::string::npos,
        "the inventory cascade still closes Cargo");
    check(appearance.find("m_sFamiliar = 0;") == std::string::npos &&
        inventoryPackets.find("m_sFamiliar = 0;") == std::string::npos &&
        selectChar.find("m_sFamiliar = 0;") == std::string::npos,
        "no equipment projection discards the slot-13 familiar");
    return failures;
}
