#pragma once

namespace ground_item_collision {
// Open fixtures restore the terrain sample, not absolute sea level. The
// immutable window must be captured before any fixture masks are applied.
inline int ApplyMask(const int (&mask)[6][6], int worldX, int worldY,
    char* heights, const char* baseHeights, int originX, int originY,
    int width, int height, int maskHeight)
{
    if (!heights || width < 3 || height < 3)
        return 0;

    for (int y = 0; y < 6; ++y)
    {
        for (int x = 0; x < 6; ++x)
        {
            const int localX = worldX + x - 2 - originX;
            const int localY = worldY + y - 2 - originY;
            if (localX < 1 || localY < 1 || localX > width - 2 || localY > height - 2)
                break;

            if (mask[y][x])
            {
                const int offset = localX + width * localY;
                heights[offset] = maskHeight == 0 && baseHeights
                    ? baseHeights[offset] : static_cast<char>(maskHeight);
            }
        }
    }
    return 1;
}
}
