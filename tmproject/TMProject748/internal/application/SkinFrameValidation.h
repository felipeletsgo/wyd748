#pragma once
#include <cstddef>
#include <cstdint>

namespace skin_frames
{
    // Frame IDs address an 80-slot lookup. Each newly allocated frame may only
    // reference an earlier parent, so recursive traversal cannot form a cycle.
    // The shipped an01/an02 files repeat IDs with different parents: these are
    // distinct owned frames, while the lookup retains the last occurrence.
    inline bool Valid(const std::uint32_t* pairs, std::size_t bytes, std::size_t count,
        std::size_t parts, const char* name, std::size_t nameCapacity)
    {
        if (!pairs || !count || count > 80 || bytes != count * 8 || parts > 8 || !name)
            return false;
        std::size_t length = 0;
        while (length < nameCapacity && name[length]) ++length;
        if (!length || length > 115 || length == nameCapacity) return false;
        bool present[80]{};
        present[0] = true; // Synthetic root used by the existing renderer.
        for (std::size_t i = 0; i < count; ++i)
        {
            const auto parent = pairs[2 * i] == UINT32_MAX ? 0 : pairs[2 * i];
            const auto id = pairs[2 * i + 1];
            if (parent >= 80 || id >= 80 || !present[parent]) return false;
            present[id] = true;
        }
        return true;
    }
}
