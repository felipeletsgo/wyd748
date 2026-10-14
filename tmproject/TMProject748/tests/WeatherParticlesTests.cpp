#include "../internal/render/effects/weather/WeatherParticles.h"
#include "../internal/render/effects/weather/WeatherPresentation.h"
#include <cstdio>

namespace {
struct Position { float x, y, z; };
struct Vertex { Position position; unsigned int diffuse; };
struct WeatherEffect {
    int m_bShow = 1;
    int m_bVisible = 1;
    unsigned int m_dwOldServerTime = 123;
    void SetActive(bool active) { weather_presentation::SetActive(*this, active); }
};
}

int RunWeatherParticlesTests(int& checks)
{
    using namespace weather_area;
    int failures = 0;
    auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) { ++failures; std::printf("FAIL weather particles: %s\n", name); }
    };

    check(HalfSize(6) == 6.0f && HalfSize(0) == 6.0f && HalfSize(12) == 12.0f && HalfSize(99) == 20.0f,
        "the area half size is bounded and 6 keeps the classic area");
    check(ParticleCount(kNativeRainDrops, 6.0f, kMaxRainDrops) == 50 &&
        ParticleCount(kNativeSnowFlakes, 6.0f, kMaxSnowFlakes) == 200,
        "the classic area keeps the native counts");
    check(ParticleCount(kNativeRainDrops, 12.0f, kMaxRainDrops) == 200 &&
        ParticleCount(kNativeSnowFlakes, 12.0f, kMaxSnowFlakes) == 800,
        "doubling the side keeps the density with four times the particles");
    check(ParticleCount(kNativeSnowFlakes, 20.0f, kMaxSnowFlakes) == kMaxSnowFlakes,
        "the count never exceeds the batch capacity");

    bool nativeRange = true;
    for (int r = 0; r < 24; ++r)
        nativeRange = nativeRange && RandomOffset(6.0f, r) == 6.0f - static_cast<float>(r) / 2.0f;
    check(nativeRange, "half size 6 reproduces 6 - (rand() % 24) / 2");
    bool wideRange = true;
    for (int r = 0; r < 1000; ++r) {
        const float v = RandomOffset(12.0f, r);
        wideRange = wideRange && v > -12.0f && v <= 12.0f;
    }
    check(wideRange && RandomOffset(12.0f, 47) == -11.5f, "the wide area covers both sides evenly");

    check(AttributeCell(-3.0f) == 0 && AttributeCell(4096.0f) == 0 && AttributeCell(40.0f) == 10,
        "attribute cells stay inside the 1024x1024 map");
    check(InSplashArea(6.0f, -6.0f) && !InSplashArea(6.5f, 0.0f),
        "splashes keep the native area");

    Vertex corners[4] = { {{-1, 1, 0}, 1}, {{1, 1, 0}, 2}, {{1, -1, 0}, 3}, {{-1, -1, 0}, 4} };
    Vertex batch[12]{};
    AppendQuad(batch, 1, corners, 10.0f, 5.0f, 20.0f);
    check(batch[6].diffuse == 1 && batch[7].diffuse == 2 && batch[8].diffuse == 3 &&
        batch[9].diffuse == 1 && batch[10].diffuse == 3 && batch[11].diffuse == 4,
        "a quad becomes two triangles with the fan order");
    check(batch[6].position.x == 9.0f && batch[6].position.y == 6.0f && batch[6].position.z == 20.0f &&
        batch[0].diffuse == 0, "quads are moved to their world position at their own slot");
    using namespace weather_presentation;
    WeatherEffect rain, snow, heavy;
    Apply(kWeatherClear, &rain, &snow, &heavy);
    check(!rain.m_bShow && !snow.m_bShow && !heavy.m_bShow &&
        !rain.m_bVisible && !snow.m_bVisible && !heavy.m_bVisible,
        "clear bootstrap disables all three inherited-visible effects");
    for (int from = 0; from <= 3; ++from) {
        for (int to = 0; to <= 3; ++to) {
            Apply(from, &rain, &snow, &heavy);
            Apply(to, &rain, &snow, &heavy);
            check(rain.m_bShow == (to == 1) && snow.m_bShow == (to >= 2) &&
                heavy.m_bShow == (to == 3) && !(rain.m_bShow && (snow.m_bShow || heavy.m_bShow)),
                "every weather transition selects exclusive rain or snow, including clear");
        }
    }
    Apply(kWeatherSnow, static_cast<WeatherEffect*>(nullptr), &snow, static_cast<WeatherEffect*>(nullptr));
    check(snow.m_bShow == 1, "selection tolerates partially initialized scenes");
    snow.m_dwOldServerTime = 400;
    snow.SetActive(true);
    check(snow.m_dwOldServerTime == 400, "repeated weather does not restart the particle clock");
    snow.SetActive(false);
    snow.m_bVisible = 1; // Simulate an unrelated generic visibility update.
    check(!snow.m_bShow, "generic visibility cannot reactivate the weather activation bit");
    snow.SetActive(true);
    check(snow.m_dwOldServerTime == 0, "reactivation starts with a fresh particle clock");
    check(Elapsed(123456, 0) == 0 && Elapsed(1016, 1000) == 16 &&
        Elapsed(100000, 1000) == 100 && Elapsed(0x10u, 0xFFFFFFF0u) == 32,
        "particle clocks handle first ticks, long pauses and unsigned wrap");
    check(OwnsSnowSound(0.05f) && !OwnsSnowSound(0.1f),
        "only the base snow layer owns the shared ambient sound");
    check(SkyTransition(1, 0) == 11 && SkyTransition(2, 11) == -1 &&
        SkyTransition(0, 1) == 10 && SkyTransition(0, 10) == -1 &&
        SkyTransition(0, 2) == -1 && SkyTransition(0, 13) == -1,
        "sky blends change once and preserve unrelated clear day/night states");

    for (int weather = 0; weather <= 3; ++weather) {
        for (int x = 0; x < 32; ++x) {
            for (int y = 0; y < 32; ++y) {
                for (int dungeon = 0; dungeon <= 4; ++dungeon) {
                    int expected = weather;
                    if (dungeon == 1 || dungeon == 2 || ((x == 29 || x == 30) && y == 22))
                        expected = 0;
                    else if ((dungeon == 3 || dungeon == 4) && weather >= 2)
                        expected = 1;
                    else if (x >= 27 && x <= 30 && y >= 21 && y <= 24)
                        expected = weather == 1 ? 3 : 2;
                    check(Resolve(weather, x, y, dungeon, false) == expected,
                        "regional and dungeon presentation matches the native selection rules");
                    check(Resolve(weather, x, y, dungeon, true) == 0,
                        "the aura overrides all regional precipitation");
                }
            }
        }
        check(Resolve(weather, 10, 10, 5, false) == 0,
            "other indoor terrain suppresses precipitation");
        check(Resolve(weather, 10, 10, 0, false) == weather,
            "leaving an indoor area restores the unchanged server weather");
    }
    check(Resolve(-1, 27, 21, 0, false) == 0 && Resolve(4, 10, 10, 0, false) == 0,
        "invalid weather is never displayed as precipitation");
    return failures;
}
