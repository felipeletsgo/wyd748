#include "../internal/render/effects/weather/WeatherParticles.h"
#include <cstdio>

namespace {
struct Position { float x, y, z; };
struct Vertex { Position position; unsigned int diffuse; };
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
    return failures;
}
