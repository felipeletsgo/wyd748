#pragma once

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <system_error>

// A development-only table can select a local server without changing the
// versioned 7.48 asset. A malformed override must fail closed, not silently
// connect to the distribution endpoint.
inline std::FILE* WYD748_OpenServerListAsset(const char* localPath, const char* standardPath)
{
    std::FILE* file = nullptr;
    std::error_code error;
    const bool localExists = std::filesystem::exists(localPath, error);
    if (error)
        return nullptr;
    if (localExists) {
        fopen_s(&file, localPath, "rb");
        return file;
    }
    fopen_s(&file, standardPath, "rb");
    return file;
}

// The 7.48 serverlist is a fixed-size table. Never expose a partially read
// table to the endpoint decoder.
template <size_t Groups, size_t Slots, size_t Width>
bool WYD748_ReadServerList(std::FILE* file, char (&servers)[Groups][Slots][Width])
{
    static_assert(Width == 64, "7.48 serverlist entries are 64 bytes");
    std::memset(servers, 0, sizeof servers);
    if (!file)
        return false;
    if (std::fread(servers, 1, sizeof servers, file) != sizeof servers ||
        std::fgetc(file) != EOF || std::ferror(file)) {
        std::memset(servers, 0, sizeof servers);
        return false;
    }
    return true;
}
