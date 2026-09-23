#pragma once

#include <cstddef>
#include <climits>

// Pure query over a borrowed occupancy buffer; does not change items or ownership.
namespace grid_occupancy
{
    // Clip writes at the right/bottom edges for equipment receptacles smaller
    // than the footprint. Overlap is not checked. False means rejection without
    // a write; the grid still owns the buffer.
    inline bool FillClipped(int* cells, int columns, int rows,
        int x, int y, int width, int height, int value)
    {
        if (!cells || columns <= 0 || rows <= 0 || columns > INT_MAX / rows ||
            x < 0 || y < 0 || x >= columns || y >= rows ||
            width <= 0 || height <= 0 || width > INT_MAX - x || height > INT_MAX - y)
            return false;
        const int clippedWidth = width < columns - x ? width : columns - x;
        const int clippedHeight = height < rows - y ? height : rows - y;
        for (int dy = 0; dy < clippedHeight; ++dy)
            for (int dx = 0; dx < clippedWidth; ++dx)
                cells[(y + dy) * columns + x + dx] = value;
        return true;
    }

    inline bool ContainsRectangle(int columns, int rows, int x, int y, int width, int height)
    {
        // Validate before addition/multiplication so invalid pointer coordinates
        // or dimensions cannot become negative indices or signed overflow.
        return columns > 0 && rows > 0 && columns <= INT_MAX / rows &&
            x >= 0 && y >= 0 && x < columns && y < rows &&
            width > 0 && height > 0 && width <= columns - x && height <= rows - y;
    }

    // The caller supplies columns*rows cells. Only value 1 blocks placement,
    // preserving the legacy rule. Geometric rejection does not read the buffer.
    inline bool CanPlace(const int* cells, int columns, int rows,
        int x, int y, int width, int height)
    {
        if (!cells || !ContainsRectangle(columns, rows, x, y, width, height))
            return false;
        for (int dy = 0; dy < height; ++dy)
            for (int dx = 0; dx < width; ++dx)
                if (cells[(y + dy) * columns + x + dx] == 1)
                    return false;
        return true;
    }
}

// Pure visual-list boundary: does not decide geometry, occupancy, or wire.
// Capacity comes from the actual array. Rejection leaves ownership with the caller.
namespace grid_insertion
{
    template <typename Item, std::size_t Capacity>
    bool CanAppend(Item* const (&)[Capacity], int count, const Item* item)
    {
        return item != nullptr && count >= 0 &&
            static_cast<std::size_t>(count) < Capacity;
    }

    // Execute legacy insertion once, only when the list has room.
    // The callback owns every mutation, including count and binding.
    // No pointer is retained/released here, and failures do not trigger retries.
    template <typename Item, std::size_t Capacity, typename Insert>
    int Execute(Item* const (&list)[Capacity], int count, Item* item, Insert&& insert)
    {
        if (!CanAppend(list, count, item))
            return 0;
        return insert();
    }
}
