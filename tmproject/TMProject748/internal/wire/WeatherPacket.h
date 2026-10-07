#pragma once

#include <cstddef>

// 0x18B world weather (native receiver FUN_00492e7d): MSG_STANDARDPARM with
// the weather in Parm. The same value arrives in MSG_CNFCharacterLogin.Weather.
constexpr auto MSG_Weather_Opcode = 0x18B;
constexpr std::size_t kWeatherPacketSize = 16;

constexpr int kWeatherClear = 0;
constexpr int kWeatherRain = 1;
constexpr int kWeatherSnow = 2;
constexpr int kWeatherHeavySnow = 3;

inline bool IsWeatherValue(int weather)
{
    return weather >= kWeatherClear && weather <= kWeatherHeavySnow;
}

// Weather passed to BASE_GetSkillDamage for a caster in field (fieldX,
// fieldY) = position >> 7. Heavy snow counts as snow and the 27-30 x 21-24
// fields always count as snow. WYD-Go applies the same rule
// (skillWeatherAt), so displayed and dealt damage agree.
inline int SkillDamageWeather(int weather, int fieldX, int fieldY)
{
    if (weather == kWeatherHeavySnow)
        weather = kWeatherSnow;
    if (fieldX > 26 && fieldX < 31 && fieldY > 20 && fieldY < 25)
        weather = kWeatherSnow;
    return weather;
}
