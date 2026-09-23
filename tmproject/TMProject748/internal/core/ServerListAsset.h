#pragma once

#include <cstdio>
#include <cstring>

// The 7.48 serverlist is a fixed-size table. Never expose a partially read
// table to the endpoint decoder.
template <size_t Groups, size_t Slots, size_t Width>
bool WYD748_ReadServerList(std::FILE* file, char (&servers)[Groups][Slots][Width])
{
    static_assert(Width == 64, "7.48 serverlist entries are 64 bytes");
    std::memset(servers, 0, sizeof servers);
    if (!file)
        return false;
    if (std::fread(servers, 1, sizeof servers, file) != sizeof servers) {
        std::memset(servers, 0, sizeof servers);
        return false;
    }
    return true;
}
