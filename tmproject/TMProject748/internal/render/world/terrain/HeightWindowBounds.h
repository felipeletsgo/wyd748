#pragma once

// Bounds of the scene height window and world-coordinate tables used by the
// BasedefWorld.cpp helpers (C-25). Header-only and device-free.
namespace height_window
{
// Index of world cell (x, y) in the window that starts at (posX, posY) and
// spans width x height cells, or -1 outside it. Same layout as
// pHeight[x + width * (y - posY) - posX].
constexpr int Index(int x, int y, int posX, int posY, int width, int height)
{
    const long long column = static_cast<long long>(x) - posX;
    const long long row = static_cast<long long>(y) - posY;
    if (width <= 0 || height <= 0 || column < 0 || row < 0 || column >= width || row >= height)
        return -1;
    return static_cast<int>(column + static_cast<long long>(width) * row);
}

// Missing height data blocks a route, as an impassable cell does.
inline int Sample(const char* data, int x, int y, int posX, int posY, int width, int height)
{
    const int index = Index(x, y, posX, posY, width, height);
    return data && index >= 0 ? data[index] : 127;
}
}

namespace world_zone
{
// Inclusive rectangle test. BASE_GetVillage compared only x >= vx1 and tested
// the other three bounds for truthiness.
constexpr bool Contains(int x, int y, int x1, int y1, int x2, int y2)
{
    return x >= x1 && x <= x2 && y >= y1 && y <= y2;
}
}

namespace attribute_map
{
constexpr int kCellsPerSide = 1024;
constexpr int kCellSize = 4;

// Row or column of g_pAttribute for a world coordinate, or -1 for a negative
// coordinate (the legacy `c / 4 % 1024` produced a negative subscript).
constexpr int Cell(int coordinate)
{
    return coordinate < 0 ? -1 : (coordinate / kCellSize) % kCellsPerSide;
}
}
