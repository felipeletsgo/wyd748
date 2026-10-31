#pragma once
#include <cmath>

namespace cannon_targeting {
// Compare directions across the zero-angle seam, retaining the native half-plane.
inline bool InFiringArc(float facing, float target)
{
    return std::cos(target - facing) >= -0.000001f;
}
}
