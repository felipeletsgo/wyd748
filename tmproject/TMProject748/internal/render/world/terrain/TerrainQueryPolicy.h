#pragma once

// Pure decisions of TMGround queries and rendering. No device dependency.

namespace terrain_tile_samples
{
constexpr int kTilesPerSide = 64;
constexpr int kTileCount = kTilesPerSide * kTilesPerSide;

// Tile-map indices of the four corners of tile (x, y) for a quad that spans
// `tickX` by `tickY` tiles: i00 = (x, y), i10 = (x + tickX, y),
// i01 = (x, y + tickY), i11 = (x + tickX, y + tickY). A corner past the last
// row or column repeats that row or column, so the block edge is flat in
// each axis independently, including the (63, 63) corner (C-05, A25-03,
// A25-06). Returns false outside [0, 63] x [0, 63].
struct Quad
{
    int i00;
    int i10;
    int i01;
    int i11;
};

constexpr int Neighbor(int coordinate, int tick)
{
    const int next = coordinate + (tick > 0 ? tick : 0);
    return next < kTilesPerSide ? next : kTilesPerSide - 1;
}

constexpr bool Resolve(int x, int y, int tickX, int tickY, Quad& quad)
{
    if (x < 0 || y < 0 || x >= kTilesPerSide || y >= kTilesPerSide)
        return false;
    const int x1 = Neighbor(x, tickX);
    const int y1 = Neighbor(y, tickY);
    quad.i00 = x + y * kTilesPerSide;
    quad.i10 = x1 + y * kTilesPerSide;
    quad.i01 = x + y1 * kTilesPerSide;
    quad.i11 = x1 + y1 * kTilesPerSide;
    return true;
}
}

namespace terrain_quake
{
constexpr unsigned int kDurationMs = 2000;

// A quake moves the effect center; the ground block origin stays fixed.
template<class Ground, class Position>
void Start(Ground& ground, unsigned int now, const Position& center)
{
    ground.m_dwEffStart = now;
    ground.m_vecEffset = center;
}

// Vertical amplitude of the ground shake. The legacy expression subtracted
// 2000 in unsigned arithmetic, so the amplitude was about 1.7e7 instead of a
// damped -4 * height (A25-01). The elapsed time is a signed tick difference;
// a start slightly ahead of the frame time counts as elapsed 0. Returns false
// when no shake is active.
constexpr bool Amplitude(unsigned int now, unsigned int start, float effectHeight, float& amplitude)
{
    amplitude = 0.0f;
    if (start == 0)
        return false;
    int elapsed = static_cast<int>(now - start);
    if (elapsed >= static_cast<int>(kDurationMs))
        return false;
    if (elapsed < 0)
        elapsed = 0;
    amplitude = 4.0f * effectHeight * static_cast<float>(elapsed - static_cast<int>(kDurationMs)) /
        static_cast<float>(kDurationMs);
    return true;
}
}

namespace terrain_walk_type
{
// Footstep class of a front texture index (TMGround::GetTileType after the
// attribute test). The four water and road ranges return 0; the legacy test
// joined them with AND and never matched (A25-04).
constexpr int ForTexture(int textureIndex)
{
    if ((textureIndex >= 14 && textureIndex <= 17) ||
        (textureIndex >= 38 && textureIndex <= 77) ||
        (textureIndex >= 86 && textureIndex <= 101) ||
        (textureIndex >= 130 && textureIndex <= 149))
        return 0;
    if (textureIndex >= 186 && textureIndex <= 193)
        return 11;
    if (textureIndex >= 202 && textureIndex <= 205)
        return 8;
    if (textureIndex < 230 || textureIndex > 231)
        return 3;
    return 9;
}
}
