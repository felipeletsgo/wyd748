#pragma once

// Compatible CC extension: only schedules existing 7.48 item/attack intentions.
namespace cc_mode
{
    constexpr int TextureSlot = 511;
    constexpr int FirstTextureSet = 550;
    constexpr int Fixed = 2;
    constexpr int Cyclic = 1;
    constexpr int Free = 0;

    constexpr int NormalizeThreshold(int value)
    {
        return value < 0 ? 0 : value > 90 ? 90 : value - value % 10;
    }

    constexpr int NextThreshold(int value)
    {
        return (NormalizeThreshold(value) + 10) % 100;
    }

    constexpr bool BelowThreshold(int current, int maximum, int percent)
    {
        return percent > 0 && current > 0 && maximum > 0 &&
            static_cast<long long>(current) * 100 < static_cast<long long>(maximum) * percent;
    }

    constexpr bool ShouldFeed(int hp, int maximum, int hunger, int percent)
    {
        return percent > 0 && hp > 0 &&
            (BelowThreshold(hp, maximum, percent) || (hunger > 0 && hunger < 6));
    }
}
