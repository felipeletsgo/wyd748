#pragma once

#include <cmath>
#include <cstdint>

namespace native_inventory_glow
{
// Native 7.48 FUN_0040d13e, GeomControl +0x15c (item object +0x1c0).
inline std::uint32_t Background(int item, int type, unsigned char firstEffect)
{
    std::uint32_t color = 0;
    if (item == 4141) color = 0xFF8800BBu;
    if (item == 4142) color = 0xFF444488u;
    if (item == 1739 || item == 4145 || (type == 206 && firstEffect == 59))
        color = 0xFFAA8888u;
    if (item >= 1744 && item <= 1751)
    {
        // Native palette at 0x005b129c, in catalog order.
        constexpr std::uint32_t palette[] = {
            0xFF777777u, 0xFF666677u, 0xFF555599u, 0xFF995555u,
            0xFF559955u, 0xFF999900u, 0xFF998866u, 0xFF775599u};
        color = palette[item - 1744];
    }
    if ((item >= 1760 && item <= 1763) || item == 4032 ||
        (item >= 5334 && item <= 5338))
        color = 0xFFFFFFAAu;
    else if (item == 3140 || (item >= 3407 && item <= 3416))
        color = 0xFFFFFFFFu;
    // Project extension, not native: the ankhs keep in the inventory the hue
    // their ground effect already has (TMItem), using native palette shades:
    // Justice (MP) red, Eternity (HP) blue, Glory (critical) green.
    if (item == 661)
        color = 0xFF995555u;
    else if (item == 662)
        color = 0xFF555599u;
    else if (item == 663)
        color = 0xFF559955u;
    return color;
}

// FUN_0042eeea: sine pulse, 2000 ms period; alpha is not modulated.
inline std::uint32_t PulsedColor(std::uint32_t color, std::uint32_t elapsed)
{
    constexpr float pi = 3.1415927410125732421875f;
    const float phase = ((static_cast<float>(elapsed % 2000u) * 0.0005f - 0.5f)
        * 2.0f) * pi;
    const float ratio = std::sin(phase) * 0.35f + 0.65f;
    const auto channel = [ratio](std::uint32_t value) {
        return static_cast<std::uint32_t>(static_cast<float>(value & 0xFFu) * ratio);
    };
    return (color & 0xFF000000u) | (channel(color >> 16) << 16) |
        (channel(color >> 8) << 8) | channel(color);
}
}
