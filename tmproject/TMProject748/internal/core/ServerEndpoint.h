#pragma once

#include <array>
#include <cstddef>
#include <cstring>

// serverlist.bin stores every endpoint in a fixed-width cell. Copy only text
// that is fully terminated inside that cell so a malformed asset cannot make
// the client read into the next channel entry.
template <std::size_t DestinationSize, std::size_t SourceSize>
inline bool CopyServerEndpoint(char (&destination)[DestinationSize], const char (&source)[SourceSize])
{
    static_assert(DestinationSize >= SourceSize, "Server endpoint destination too small");
    const auto* terminator = static_cast<const char*>(std::memchr(source, '\0', SourceSize));
    if (!terminator || terminator == source) {
        destination[0] = '\0';
        return false;
    }

    std::memcpy(destination, source, static_cast<std::size_t>(terminator - source) + 1);
    return true;
}

// Channel zero holds each group's status endpoint. The asset has a fixed number of
// groups, so a completely populated table has no empty sentinel after it.
template <std::size_t GroupCount, std::size_t ChannelCount, std::size_t CellSize>
inline int LastConfiguredServerGroup(const char (&serverList)[GroupCount][ChannelCount][CellSize])
{
    int lastGroup = -1;
    for (std::size_t group = 0; group < GroupCount; ++group) {
        if (!serverList[group][0][0])
            break;
        lastGroup = static_cast<int>(group);
    }
    return lastGroup;
}

// The server selector inserts rows in descending sn.bin order, skipping entries
// whose group has no status endpoint. Keep event rows tied to those exact slots.
template <std::size_t OrderedCount, std::size_t GroupCount, std::size_t ChannelCount, std::size_t CellSize>
inline auto VisibleServerGroupSlots(const int (&orderedGroups)[OrderedCount],
    const char (&serverList)[GroupCount][ChannelCount][CellSize], int lastSlot)
{
    struct Result {
        std::array<int, GroupCount> slots{};
        int count = 0;
    } result;
    if (lastSlot >= static_cast<int>(OrderedCount))
        lastSlot = static_cast<int>(OrderedCount) - 1;
    if (lastSlot >= static_cast<int>(GroupCount))
        lastSlot = static_cast<int>(GroupCount) - 1;
    for (int slot = lastSlot; slot >= 0; --slot) {
        const int group = orderedGroups[slot] - 1;
        if (group >= 0 && static_cast<std::size_t>(group) < GroupCount && serverList[group][0][0])
            result.slots[result.count++] = slot;
    }
    return result;
}

// A configured group can have gaps between channel endpoints. List rows are
// compacted when drawn, so their ordinal must not become the endpoint index.
template <std::size_t Count>
inline int ChannelForVisibleRow(const int (&channelSlots)[Count], int visibleCount, int row)
{
    if (visibleCount < 0 || visibleCount > static_cast<int>(Count) ||
        row < 0 || row >= visibleCount)
        return -1;
    return channelSlots[row];
}

template <std::size_t GroupCount, std::size_t ChannelCount, std::size_t CellSize>
inline const char* ServerChannelNameAt(const char (&names)[GroupCount][ChannelCount][CellSize], int group, int channel)
{
    if (group < 0 || static_cast<std::size_t>(group) >= GroupCount ||
        channel < 0 || static_cast<std::size_t>(channel) >= ChannelCount)
        return nullptr;

    const char* name = names[group][channel];
    return name[0] && std::memchr(name, '\0', CellSize) ? name : nullptr;
}

template <std::size_t DestinationSize, std::size_t GroupCount, std::size_t ChannelCount, std::size_t CellSize>
inline bool CopyServerEndpointAt(char (&destination)[DestinationSize],
    const char (&serverList)[GroupCount][ChannelCount][CellSize], int group, int channel)
{
    if (group < 0 || static_cast<std::size_t>(group) >= GroupCount ||
        channel < 1 || static_cast<std::size_t>(channel) >= ChannelCount ||
        !CopyServerEndpoint(destination, serverList[group][channel])) {
        destination[0] = '\0';
        return false;
    }
    return true;
}
