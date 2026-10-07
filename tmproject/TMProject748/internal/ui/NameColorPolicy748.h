#pragma once
#include <cstdint>

namespace name_color_748 {
struct Style { std::uint32_t color; bool border; };

// FUN_004fff0b interpolates the native palette with truncation, not rounding.
// NPC white is an explicit product policy: native spawns use raw CP 75 instead.
template<class Color>
inline Style Calculate(unsigned int entityID, unsigned int rawCP,
    const Color* palette)
{
    if (entityID >= 1000)
        return {0xFFFFFFFFu, false};
    if (rawCP < 10)
        return {0xFF000000u, true};
    if (rawCP > 150)
        rawCP = 150;
    const unsigned int index = (rawCP - 5) / 20;
    const float fraction = static_cast<float>(rawCP - (index * 20 + 5)) / 20.0f;
    std::uint32_t color = 0xFF000000u;
    for (unsigned int shift = 0; shift <= 16; shift += 8) {
        const float left = static_cast<float>((palette[index] >> shift) & 255u);
        const float right = static_cast<float>((palette[index + 1] >> shift) & 255u);
        color |= static_cast<unsigned int>(left * (1.0f - fraction) + right * fraction) << shift;
    }
    return {color, false};
}
}
