#pragma once

#include <cmath>

namespace camera_zoom {
inline constexpr float LegacyDistance = 20.0f;
inline constexpr float FieldDefaultDistance = 22.0f;
inline constexpr float FieldMaximumDistance = 24.0f;

// Ground tiles span two world units. Extend fixed query/render bounds only
// beyond the previous camera limit, with at most two additional tile rings.
inline int GroundTilePadding(float distance)
{
    if (distance <= LegacyDistance)
        return 0;
    if (distance >= FieldMaximumDistance)
        distance = FieldMaximumDistance;
    return static_cast<int>(std::ceil((distance - LegacyDistance) / 2.0f));
}
}
