#pragma once

// Area and density of the rain and snow particles. The 7.48 effects keep 50
// drops and 200 flakes in a 12x12-unit square around the player, which leaves
// the sides of the screen dry. The area is now configurable (ClientSettings
// WeatherArea, half size in world units) and the count grows with it so the
// density stays the same; the particles are drawn in one batched call.
namespace weather_area {

constexpr float kNativeHalfSize = 6.0f;
constexpr int kNativeRainDrops = 50;
constexpr int kNativeSnowFlakes = 200;
constexpr int kMaxRainDrops = 300;
constexpr int kMaxSnowFlakes = 1000;

inline float HalfSize(int setting)
{
    if (setting < 6)
        return kNativeHalfSize;
    return setting > 20 ? 20.0f : static_cast<float>(setting);
}

// Native count scaled by the area, between the native count and the cap.
inline int ParticleCount(int nativeCount, float halfSize, int maxCount)
{
    const float ratio = halfSize / kNativeHalfSize;
    int count = static_cast<int>(static_cast<float>(nativeCount) * ratio * ratio + 0.5f);
    if (count < nativeCount)
        count = nativeCount;
    return count > maxCount ? maxCount : count;
}

// Offset in [-half, half) on the native half-unit grid; at half 6 this is the
// native "6 - (rand() % 24) / 2".
inline float RandomOffset(float halfSize, int random)
{
    int steps = static_cast<int>(halfSize * 4.0f);
    if (steps < 1)
        steps = 1;
    if (random < 0)
        random = -random;
    return halfSize - static_cast<float>(random % steps) / 2.0f;
}

// g_pAttribute cell of a world coordinate; out-of-map cells use cell 0 as the
// native renderer does.
inline int AttributeCell(float worldCoordinate)
{
    const int cell = static_cast<int>(worldCoordinate) / 4;
    return cell < 0 || cell >= 1024 ? 0 : cell;
}

// Ground splashes stay in the native area so their number does not grow.
inline bool InSplashArea(float dx, float dz)
{
    return dx >= -kNativeHalfSize && dx <= kNativeHalfSize &&
        dz >= -kNativeHalfSize && dz <= kNativeHalfSize;
}

// Appends one quad (triangle list, 6 vertices) of `corners` moved to
// (x, y, z). Vertex is any type with a `position` member holding x, y, z.
template <typename Vertex>
void AppendQuad(Vertex* out, int quadIndex, const Vertex (&corners)[4], float x, float y, float z)
{
    static constexpr int order[6] = { 0, 1, 2, 0, 2, 3 };
    Vertex* target = out + quadIndex * 6;
    for (int i = 0; i < 6; ++i)
    {
        target[i] = corners[order[i]];
        target[i].position.x += x;
        target[i].position.y += y;
        target[i].position.z += z;
    }
}

} // namespace weather_area
