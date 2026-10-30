#include "SourceMethod.h"
#include "../internal/core/CameraInput.h"
#include "../internal/game/entities/BerielAuraPolicy.h"
#include "../internal/ui/controls/ScrollbarLayout.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <windows.h>

namespace {
std::string ReadClientSource(const char* relative)
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

// Field input, scrollbar geometry, camera zoom and the BerialGuard aura.
int RunFieldViewContractTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool passed, const char* message) {
        ++checks;
        if (!passed) {
            ++failures;
            std::fprintf(stderr, "FAIL field view: %s\n", message);
        }
    };

    // Explicit project choice: 1-0 selects skills instead of native F1-F10.
    // Native FUN_00454763 still establishes visible-list page scrolling.
    const auto fieldInput = ReadClientSource("internal/app/scenes/TMFieldSceneInput.cpp");
    const auto keydown = source_contract::Method(fieldInput,
        "int TMFieldScene::OnKeyDownEvent(unsigned int iKeyCode)");
    check(!keydown.empty() && keydown.find("VK_F10") == std::string::npos &&
        keydown.find("VK_F1 &&") == std::string::npos &&
        keydown.find("SelectShortSkill(slot)") == std::string::npos,
        "F1-F10 no longer select or assign skills");
    const auto fieldScene = ReadClientSource("internal/app/scenes/TMFieldScene.cpp");
    const auto characters = source_contract::Method(fieldScene,
        "int TMFieldScene::OnCharEvent(char iCharCode, int lParam)");
    check(characters.find("TMScene::OnCharEvent(iCharCode, lParam)") <
        characters.find("OnKeySkill(iCharCode, lParam)") &&
        characters.find("OnKeySkill(iCharCode, lParam)") <
        characters.find("OnKeyShortSkill(iCharCode, lParam)"),
        "focused text input precedes assignment and number-key selection");
    const auto assignment = source_contract::Method(fieldInput,
        "int TMFieldScene::OnKeySkill(char iCharCode, int lParam)");
    check(assignment.find("if (!m_pSkillPanel || !g_pCursor)") != std::string::npos &&
        assignment.find("if (!m_pSkillPanel->IsVisible())") != std::string::npos &&
        assignment.find("case '!':") != std::string::npos &&
        assignment.find("case ')':") != std::string::npos,
        "Shift+1-0 retains hovered assignment in the visible skill panel");
    const auto beltSelection = source_contract::Method(fieldInput,
        "int TMFieldScene::SelectShortSkill(int slot)");
    check(beltSelection.find("slot < 0 || slot >= 10 || !g_pObjectManager") != std::string::npos &&
        beltSelection.find("!m_pGridSkillBelt2 || !m_pGridSkillBelt3") != std::string::npos &&
        beltSelection.find("cSkillIndex < 248 && m_pMyHuman") != std::string::npos,
        "selection safely handles invalid slots and incomplete scene construction");
    check(beltSelection.find("m_cSelectShortSkill = slot;") != std::string::npos &&
        beltSelection.find("if (m_pGridSkillBelt3->IsVisible())") != std::string::npos &&
        beltSelection.find("m_cSelectShortSkill += 10;") != std::string::npos,
        "the visible second bank offsets selection by ten");
    const auto numberKeys = source_contract::Method(fieldInput,
        "int TMFieldScene::OnKeyShortSkill(char iCharCode, int lParam)");
    check(numberKeys.find("if (m_bNumPad == 1)") != std::string::npos &&
        numberKeys.find("return SelectShortSkill(slot);") != std::string::npos,
        "number keys retain numpad suppression and use the shared selection helper");
    check(numberKeys.find("slot = iCharCode - '1';") != std::string::npos &&
        numberKeys.find("if (iCharCode == '0')") != std::string::npos &&
        numberKeys.find("slot = 9;") != std::string::npos,
        "1-9 select the first nine slots and 0 selects the tenth");
    for (const auto key : {"VK_PRIOR", "VK_NEXT"}) {
        const auto scrolling = source_contract::Method(keydown,
            std::string("if (iKeyCode == ") + key + ")");
        check(scrolling.find("m_pMsgPanel && m_pMsgPanel->IsVisible() == 1") != std::string::npos &&
            scrolling.find("pChatList = m_pMsgList;") != std::string::npos &&
            scrolling.find("pChatList = m_pChatList;") != std::string::npos,
            "page keys choose messages only when their panel is visible");
        check(scrolling.find("pChatList && pChatList->IsVisible() == 1 && pChatList->m_pScrollBar") != std::string::npos &&
            scrolling.find(key == std::string("VK_PRIOR") ? "->Up();" : "->Down();") != std::string::npos &&
            scrolling.find("IsVisible() == 0") == std::string::npos,
            "page keys scroll the visible list in the requested direction");
    }

    for (const float scale : {0.75f, 1.0f, 1.2f, 1.5f, 2.0f}) {
        const float resourceHeight = 320.0f;
        const float created = scrollbar_layout::DownArrowY(resourceHeight, 1.0f) * scale;
        const float resized = scrollbar_layout::DownArrowY(resourceHeight * scale, scale);
        check(created - resized < 0.001f && resized - created < 0.001f,
            "UI2 scrollbar creation and pixel resize apply UI scale exactly once");
        check(resized == 310.0f * scale,
            "the down arrow preserves the native ten-unit bottom inset at every scale");
        check(scrollbar_layout::DownArrowY(400.0f * scale, scale) > resized,
            "growing the chat moves the down arrow with the content bottom");
    }

    // [CAMERAROTATE] 1 inverts the wheel delta but not viewchange; with
    // [CAMERAVIEW] 0 the distance must follow the selected direction or the
    // camera locks at its maximum distance.
    const auto input = ReadClientSource("internal/core/EventTranslator.cpp");
    const auto camera = source_contract::Method(input, "int EventTranslator::CameraEventData()");
    check(!camera.empty(), "camera input method is available");
    check(camera.find("CameraZoomDistance(pCamera->m_fSightLength,") != std::string::npos &&
        camera.find("g_pApp->m_nCameraView, wheel, viewchange)") != std::string::npos &&
        CameraZoomDistance(10.0f, 1.2f, 10.0f, 0, 120, -120) == 9.5f &&
        CameraZoomDistance(5.0f, 1.2f, 10.0f, 1, 120, -120) == 5.5f,
        "zoom distance follows the selected wheel direction");
    check(camera.find("m_stScore.Con") == std::string::npos,
        "the zoom near limit does not grow with player CON");
    check(CameraZoomDistance(20.0f, 1.2f, 20.0f, 1, -120, 0) == 19.5f,
        "a fixed near limit lets a high-CON player zoom in from the maximum");

    check(beriel_aura::InRange(0.0f, 0.0f) && beriel_aura::InRange(8.0f, 0.0f) &&
        beriel_aura::InRange(5.0f, 6.0f), "the aura reaches players near the guard");
    check(!beriel_aura::InRange(8.1f, 0.0f) && !beriel_aura::InRange(6.0f, 6.0f) &&
        !beriel_aura::InRange(-20.0f, 3.0f), "the aura stops outside the radius");
    check(!beriel_aura::Expired(0, 100000), "no aura means nothing to restore");
    check(!beriel_aura::Expired(1000, 1500) && beriel_aura::Expired(1000, 1501),
        "the aura is released after the hold time");
    check(!beriel_aura::Expired(0xFFFFFF00u, 0x00000010u), "the hold survives a timer wrap");

    const auto effects = ReadClientSource("internal/game/entities/TMHumanEffects.cpp");
    const auto keeper = source_contract::Method(effects, "void TMHuman::RenderEffect_LegendBerielKeeper(unsigned int dwServerTime)");
    const auto inRange = keeper.find("beriel_aura::InRange(");
    const auto dungeon = keeper.find("RenderDevice::m_bDungeon = 1;");
    check(inRange != std::string::npos && dungeon != std::string::npos && inRange < dungeon &&
        keeper.find("m_nWTime = 12") == std::string::npos,
        "the guard darkens the scene only inside its radius");
    // Weather sequences through the override (audit A02): the server weather
    // received last is the one that returns, whatever the aura did meanwhile.
    {
        beriel_aura::WeatherOverride weather;
        weather.SetServer(1);                       // weather A
        weather.Refresh(1000);                      // enter the aura
        check(weather.Effective() == 0, "the aura hides weather A");
        weather.SetServer(2);                       // server sends B during the aura
        check(weather.Effective() == 0, "weather B stays hidden while the aura lasts");
        check(!weather.Release(1500), "the aura holds within kHoldMs");
        check(weather.Release(1501) && weather.Effective() == 2, "A -> aura -> B -> expiry yields B");
        check(!weather.Release(9999), "the release happens once");
    }
    {
        beriel_aura::WeatherOverride weather;
        weather.SetServer(3);
        weather.Refresh(100);
        weather.SetServer(1);
        weather.SetServer(2);                       // several packets during the aura
        weather.Refresh(400);                       // a second guard refreshes the aura
        check(!weather.Release(800), "an overlapping guard extends the aura");
        check(weather.Release(901) && weather.Effective() == 2, "the last of several packets is restored");
    }
    {
        beriel_aura::WeatherOverride weather;
        weather.SetServer(1);
        weather.Refresh(0);                         // refresh at time 0 still activates
        check(weather.Active() && weather.Effective() == 0, "a refresh at time zero is active");
        check(weather.Release(1000) && weather.Effective() == 1, "expiry without a packet restores A");
        weather.Refresh(0xFFFFFF00u);
        check(!weather.Release(0x00000010u), "the hold survives a timer wrap");
    }
    const auto scene = ReadClientSource("internal/app/scenes/TMFieldScene.cpp");
    const auto frame = source_contract::Method(scene, "int TMFieldScene::FrameMove(unsigned int dwServerTime)");
    check(frame.find("m_berielAura.Release(dwServerTime)") != std::string::npos &&
        frame.find("RenderDevice::m_bDungeon = m_pGround->m_bDungeon;") != std::string::npos &&
        frame.find("SetWeather(m_berielAura.server);") != std::string::npos,
        "leaving the aura restores the ground light and reapplies the server weather");
    const auto world = ReadClientSource("internal/app/scenes/TMFieldSceneWorld.cpp");
    const auto setWeather = source_contract::Method(world, "void TMFieldScene::SetWeather(int nWeather)");
    const auto session = ReadClientSource("internal/app/scenes/TMFieldSceneSessionPackets.cpp");
    check(setWeather.find("m_berielAura.SetServer(nWeather);") != std::string::npos &&
        session.find("m_berielAura.SetServer(pStd->Weather);") != std::string::npos,
        "every server weather update reaches the override");
    const auto weatherEffects = source_contract::Method(world, "void TMFieldScene::UpdateWeatherEffects()");
    check(setWeather.find("!IsWeatherValue(nWeather)") != std::string::npos &&
        setWeather.find("UpdateWeatherEffects();") != std::string::npos &&
        weatherEffects.find("weather_presentation::Resolve(m_berielAura.server,") != std::string::npos &&
        weatherEffects.find("m_berielAura.Active()") != std::string::npos &&
        weatherEffects.find("weather_presentation::Apply(weather, m_pRain, m_pSnow, m_pSnow2)") != std::string::npos,
        "the live field consumes the tested weather policy without changing authoritative state");
    const auto ui = ReadClientSource("internal/app/scenes/TMFieldSceneUI.cpp");
    const auto initialize = source_contract::Method(ui, "int TMFieldScene::InitializeCompatFieldScene()");
    const auto ground = ReadClientSource("internal/app/scenes/TMSceneGround.cpp");
    check(initialize.find("UpdateWeatherEffects();") > initialize.find("m_pSnow2 = new TMSnow(2.0f);") &&
        initialize.find("UpdateWeatherEffects();") != std::string::npos &&
        frame.find("UpdateWeatherEffects();") < frame.find("TMScene::FrameMove(dwServerTime);") &&
        ground.find("pFieldScene->UpdateWeatherEffects();") != std::string::npos &&
        ground.find("SetWeather(g_nWeather)") == std::string::npos &&
        ground.find("g_nWeather = 0;") == std::string::npos,
        "bootstrap, live movement and terrain changes use the same selector without losing server weather");
    for (const auto relative : {"internal/render/effects/core/TMRain.cpp", "internal/render/world/sky/TMSnow.cpp"}) {
        const auto particles = ReadClientSource(relative);
        check(particles.find("m_bShow = m_bVisible = 0;") != std::string::npos &&
            particles.find("weather_presentation::SetActive(*this, active);") != std::string::npos &&
            particles.find("if (!m_bShow || !m_bVisible || IdleHidesAmbient()") != std::string::npos &&
            particles.find("weather_presentation::Elapsed(dwServerTime, m_dwOldServerTime)") != std::string::npos,
            "live particles start disabled, stop hidden audio and consume the tested clock");
    }
    const auto snowSource = ReadClientSource("internal/render/world/sky/TMSnow.cpp");
    const auto snowRender = source_contract::Method(snowSource, "int TMSnow::Render()");
    const auto snowStop = source_contract::Method(snowSource, "void TMSnow::StopAmbientSound()");
    check(snowRender.find("if (weather_presentation::OwnsSnowSound(m_fScale))\n        GetSoundAndPlayIfNot(113") != std::string::npos &&
        snowStop.find("weather_presentation::OwnsSnowSound(m_fScale) && g_pSoundManager") != std::string::npos,
        "the heavy snow layer cannot play or stop the base layer's sound");
    const auto appearance = ReadClientSource("internal/game/entities/TMHumanAppearance.cpp");
    const auto localEquip = source_contract::Method(appearance, "void TMHuman::SetPacketMOBItem(STRUCT_MOB* pMobData)");
    const auto observerEquip = source_contract::Method(appearance, "void TMHuman::SetPacketEquipItem(unsigned short* sEquip)");
    check(localEquip.find("SetCitizenMantle(m_wMantuaSkin)") != std::string::npos &&
        observerEquip.find("SetCitizenMantle(m_wMantuaSkin)") != std::string::npos,
        "both equipment paths apply the native citizenship cape mapping");
    const auto selection = ReadClientSource("internal/app/scenes/TMSelectCharScene.cpp");
    const auto citizenAssignment = selection.find("m_citizen = static_cast<unsigned char>(pSelChar->Equip[i][0].stEffect[2].cValue)");
    check(citizenAssignment != std::string::npos && citizenAssignment < selection.find("m_pHuman[i]->SetCitizenMantle"),
        "selection reads face citizenship before mapping the cape");
    const auto refresh = source_contract::Method(world, "int TMFieldScene::OnPacketCreateMobCompat(MSG_STANDARD* pStd)");
    const auto existingBegin = refresh.find("if (pExisting)");
    const auto existingEnd = refresh.find("return 1;", existingBegin);
    const auto existing = existingBegin == std::string::npos ? std::string{} :
        refresh.substr(existingBegin, existingEnd - existingBegin);
    check(existing.find("m_citizen = pCreateMob->Server") != std::string::npos &&
        existing.find("SetPacketEquipItem(pCreateMob->Equip)") != std::string::npos &&
        existing.find("InitObject") == std::string::npos && existing.find("InitPosition") == std::string::npos,
        "live citizenship refresh keeps the existing entity and movement");
    const auto physicalSource = ReadClientSource("internal/app/scenes/TMFieldSceneCombat.cpp");
    const auto physicalTick = source_contract::Method(physicalSource,
        "void TMFieldScene::UpdatePhysicalAttack(unsigned int dwServerTime)");
    const auto physicalClick = source_contract::Method(physicalSource,
        "int TMFieldScene::MobAttack(unsigned int wParam, D3DXVECTOR3 vec, unsigned int dwServerTime, TMHuman* target)");
    check(frame.find("UpdatePhysicalAttack(dwServerTime);") != std::string::npos &&
        frame.find("UpdatePhysicalAttack(dwServerTime);") > frame.find("UpdateTeleportPrompt()") &&
        physicalTick.find("field_interaction::PhysicalAttackTick(") != std::string::npos &&
        physicalTick.find("cc_mode::OwnsCombat(g_GameAuto)") != std::string::npos &&
        physicalTick.find("MobAttack(0, position, dwServerTime, target)") != std::string::npos,
        "the field retries the retained enemy through click validation after transition gates");
    check(physicalClick.find("m_pTargetHuman = pOver;") < physicalClick.find("dwServerTime <= m_dwOldAttackTime + 1000") &&
        physicalClick.find("target || pOver->m_bMouseOver == 1") != std::string::npos &&
        physicalClick.find("if (m_cAutoAttack == 1)") == std::string::npos,
        "physical clicks retain targets during cooldown without hover or CC dependency");
    const auto physicalPackets = ReadClientSource("internal/app/scenes/TMFieldSceneCombatPackets.cpp");
    const auto killed = source_contract::Method(physicalPackets,
        "int TMFieldScene::OnPacketCNFMobKill(MSG_CNFMobKill* pStd)");
    check(killed.find("if (m_pTargetHuman == pKilled)") != std::string::npos &&
        killed.find("m_pMyHuman->m_pMoveTargetHuman == pKilled") != std::string::npos,
        "any killer clears the selected victim, not an unrelated surviving target");
    const auto hpMp = source_contract::Method(ReadClientSource("internal/game/entities/TMHumanPackets.cpp"),
        "int TMHuman::OnPacketSetHpMp(MSG_SetHpMp* pStd)");
    check(hpMp.find("hp == 0") != std::string::npos && hpMp.find("mp == 0") == std::string::npos,
        "a mob HP/MP update releases the retained target only at zero HP, not at zero MP");
    check(physicalPackets.find("SkillIndex == 94) // Ethereal Explosion") != std::string::npos &&
        physicalPackets.find("SkillIndex == 86) // Ethereal Explosion") == std::string::npos,
        "Ethereal Explosion arrows belong to skill 94, not Golden Shield 86");
    return failures;
}
