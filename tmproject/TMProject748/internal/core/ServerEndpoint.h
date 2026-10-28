#pragma once

#include <array>
#include <cstddef>
#include <cstring>

// Coordinated extension: an optional :port stays inside the existing 64-byte
// endpoint cell. Legacy IPv4 entries retain the original game port. SocketManager
// uses inet_addr, so DNS names and IPv6 are deliberately rejected here.
template <std::size_t HostSize>
inline bool ResolveGameEndpoint(const char* endpoint, char (&host)[HostSize], int& port)
{
    host[0] = '\0';
    if (!endpoint) return false;
    const char* colon = std::strchr(endpoint, ':');
    const std::size_t length = colon ? static_cast<std::size_t>(colon - endpoint) : std::strlen(endpoint);
    if (!length || length >= HostSize) return false;
    unsigned octets = 0, value = 0, digits = 0;
    for (std::size_t i = 0; i <= length; ++i) {
        if (i == length || endpoint[i] == '.') {
            if (!digits || ++octets > 4) return false;
            value = digits = 0;
        } else {
            const unsigned char c = endpoint[i];
            if (c < '0' || c > '9' || ++digits > 3) return false;
            value = value * 10 + c - '0';
            if (value > 255) return false;
        }
    }
    if (octets != 4) return false;
    int selectedPort = 8281;
    if (colon) {
        selectedPort = 0;
        if (!colon[1]) return false;
        for (const char* c = colon + 1; *c; ++c) {
            if (*c < '0' || *c > '9') return false;
            selectedPort = selectedPort * 10 + (*c - '0');
            if (selectedPort > 65535) return false;
        }
        if (!selectedPort) return false;
    }
    std::memcpy(host, endpoint, length);
    host[length] = '\0';
    port = selectedPort;
    return true;
}

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

// The login selector identifies the synthetic featured group by the highest
// sn.bin order, not by the last populated status endpoint. That endpoint can
// belong to an ordinary group when the featured group is hidden. Group zero
// cannot aggregate any preceding groups.
template <std::size_t OrderedCount>
inline int AggregateServerGroupFromOrder(const int (&orderedGroups)[OrderedCount], int groupCount)
{
    int highestOrder = 0;
    for (const int order : orderedGroups) {
        if (order < 0 || order > groupCount)
            return -1;
        if (order > highestOrder)
            highestOrder = order;
    }
    return highestOrder > 1 ? highestOrder - 1 : -1;
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
