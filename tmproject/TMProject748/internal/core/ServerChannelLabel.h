#pragma once

#include <cstddef>
#include <cstring>

// Keep the existing 14-column FULL marker without reading past the label's
// terminator or relying on bytes left over from a previous channel.
inline bool AppendFullChannelLabel(char* label, std::size_t capacity)
{
    constexpr std::size_t markerColumn = 14;
    constexpr char marker[] = "FULL";
    if (!label || capacity < markerColumn + sizeof(marker))
        return false;

    const auto* end = static_cast<const char*>(std::memchr(label, '\0', capacity));
    if (!end)
        return false;

    const std::size_t length = static_cast<std::size_t>(end - label);
    if (length < markerColumn)
        std::memset(label + length, ' ', markerColumn - length);
    std::memcpy(label + markerColumn, marker, sizeof(marker));
    return true;
}
