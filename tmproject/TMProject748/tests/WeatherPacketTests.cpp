#include "../internal/wire/ReceivedPacketDispatch.h"
#include "../internal/wire/WeatherPacket.h"
#include <cstdio>

int RunWeatherPacketTests(int& checks)
{
    int failures = 0;
    auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) { ++failures; std::printf("FAIL weather packet: %s\n", name); }
    };

    check(received_packet::ExpectedSize(MSG_Weather_Opcode) == kWeatherPacketSize &&
        kWeatherPacketSize == sizeof(MSG_STANDARD) + sizeof(int),
        "0x18B is a validated 16-byte MSG_STANDARDPARM");
    check(IsWeatherValue(0) && IsWeatherValue(3) && !IsWeatherValue(-1) && !IsWeatherValue(4),
        "only clear, rain, snow and heavy snow are accepted");

    check(SkillDamageWeather(kWeatherHeavySnow, 16, 16) == kWeatherSnow,
        "heavy snow counts as snow for skill damage");
    check(SkillDamageWeather(kWeatherRain, 16, 16) == kWeatherRain,
        "rain outside the snow fields stays rain");
    check(SkillDamageWeather(kWeatherClear, 28, 22) == kWeatherSnow &&
        SkillDamageWeather(kWeatherRain, 27, 21) == kWeatherSnow,
        "the 27-30 x 21-24 fields always count as snow");
    check(SkillDamageWeather(kWeatherRain, 26, 22) == kWeatherRain &&
        SkillDamageWeather(kWeatherRain, 28, 25) == kWeatherRain,
        "the snow area bounds are exclusive");

    // Exhaust every field coordinate in the 4096-tile map for all four
    // accepted values, including both sides of every snow-region boundary.
    for (int weather = 0; weather <= 3; ++weather) {
        for (int x = 0; x < 32; ++x) {
            for (int y = 0; y < 32; ++y) {
                const bool snowField = x >= 27 && x <= 30 && y >= 21 && y <= 24;
                const int expected = snowField || weather == 3 ? 2 : weather;
                check(SkillDamageWeather(weather, x, y) == expected,
                    "weather and snow-region field matrix");
            }
        }
    }
    return failures;
}
