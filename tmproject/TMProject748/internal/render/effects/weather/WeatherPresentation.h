#pragma once

#include "../../../wire/WeatherPacket.h"

namespace weather_presentation {

// Local presentation only: never overwrite the server weather or its damage rules.
inline int Resolve(int server, int fieldX, int fieldY, int dungeon, bool aura)
{
    if (!IsWeatherValue(server) || aura || (dungeon != 0 && dungeon != 3 && dungeon != 4))
        return kWeatherClear;
    if ((fieldX == 29 || fieldX == 30) && fieldY == 22)
        return kWeatherClear;
    if ((server == kWeatherSnow || server == kWeatherHeavySnow) && (dungeon == 3 || dungeon == 4))
        return kWeatherRain;
    // The native snow region has permanent snow, even during world clear weather.
    if (fieldX >= 27 && fieldX <= 30 && fieldY >= 21 && fieldY <= 24)
        return server == kWeatherRain ? kWeatherHeavySnow : kWeatherSnow;
    return server;
}

template<class Rain, class Snow>
inline void Apply(int weather, Rain* rain, Snow* snow, Snow* heavy)
{
    if (rain) rain->SetActive(weather == kWeatherRain);
    if (snow) snow->SetActive(weather == kWeatherSnow || weather == kWeatherHeavySnow);
    if (heavy) heavy->SetActive(weather == kWeatherHeavySnow);
}

// Keep a separate activation bit: generic visibility must not enable weather.
template<class Effect>
inline void SetActive(Effect& effect, bool active)
{
    if (effect.m_bShow != static_cast<int>(active))
        effect.m_dwOldServerTime = 0;
    effect.m_bShow = effect.m_bVisible = active;
}

inline unsigned int Elapsed(unsigned int now, unsigned int previous)
{
    if (!previous)
        return 0;
    const unsigned int elapsed = now - previous;
    return elapsed > 100 ? 100 : elapsed;
}

inline bool OwnsSnowSound(float scale) { return scale <= 0.07f; }

inline int SkyTransition(int weather, int state)
{
    // Do not restart an in-flight blend or replace unrelated day/night states.
    if (weather != kWeatherClear && state % 10 != 1)
        return 11;
    if (weather == kWeatherClear && state % 10 == 1)
        return 10;
    return -1;
}

} // namespace weather_presentation
