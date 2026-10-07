#include "SourceMethod.h"
#include "../internal/game/entities/BerielAuraPolicy.h"

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

// Camera zoom and the BerialGuard darkness aura.
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

    // [CAMERAROTATE] 1 inverts the wheel delta but not viewchange; with
    // [CAMERAVIEW] 0 the distance must follow the selected direction or the
    // camera locks at its maximum distance.
    const auto input = ReadClientSource("internal/core/EventTranslator.cpp");
    const auto camera = source_contract::Method(input, "int EventTranslator::CameraEventData()");
    check(!camera.empty(), "camera input method is available");
    check(camera.find("(float)zoomDirection / 240.0f") != std::string::npos &&
        camera.find("(float)wheel / 240.0f") == std::string::npos,
        "zoom distance follows the selected wheel direction");

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
    return failures;
}
