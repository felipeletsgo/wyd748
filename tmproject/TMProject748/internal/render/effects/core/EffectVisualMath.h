#pragma once
#include <algorithm>

namespace effect_visual {
inline float LitChannel(float base, float light, float baseWeight, float lightWeight)
{
    return base * baseWeight + light * lightWeight;
}
inline unsigned int FadeArgb(unsigned int alpha, unsigned int red, unsigned int green,
    unsigned int blue, float progress)
{
    const float fade = std::clamp(progress, 0.0f, 1.0f);
    const auto channel = [fade](unsigned int value) {
        return static_cast<unsigned int>((std::min)(value, 255u) * fade);
    };
    return (channel(alpha) << 24) | (channel(red) << 16) | (channel(green) << 8) | channel(blue);
}
}
