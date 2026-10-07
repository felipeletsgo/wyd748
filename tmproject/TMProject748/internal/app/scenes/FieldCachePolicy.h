#pragma once

#include <cstddef>

// Least-recently-used slots for terrain fields the scene released. A field
// keeps its TMGround and TMObjectContainer, so a revisit skips rebuilding
// thousands of objects (about 60 ms per Field*.dat).
namespace field_cache {

constexpr int kMaxEntries = 16;

template <typename Ground, typename Objects>
struct Entry
{
    int x = -1;
    int y = -1;
    Ground* ground = nullptr;
    Objects* objects = nullptr;
    unsigned int lastUse = 0;

    bool Used() const { return ground != nullptr; }
};

// Index of the cached field (x, y) among the first `capacity` entries, or -1.
template <typename E, std::size_t N>
int Find(const E (&entries)[N], int capacity, int x, int y)
{
    for (int i = 0; i < capacity && i < static_cast<int>(N); ++i)
        if (entries[i].Used() && entries[i].x == x && entries[i].y == y)
            return i;
    return -1;
}

// Slot that receives a released field: the first free one, otherwise the
// least recently used, whose field the caller must delete first.
template <typename E, std::size_t N>
int ChooseSlot(const E (&entries)[N], int capacity)
{
    if (capacity > static_cast<int>(N))
        capacity = static_cast<int>(N);
    int oldest = 0;
    for (int i = 0; i < capacity; ++i)
    {
        if (!entries[i].Used())
            return i;
        if (entries[i].lastUse < entries[oldest].lastUse)
            oldest = i;
    }
    return oldest;
}

} // namespace field_cache
