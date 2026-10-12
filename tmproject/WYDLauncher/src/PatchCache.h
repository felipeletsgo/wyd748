#pragma once

// Local verification cache (patch.cache in the client directory). Each line
// records "<sha256> <size> <last-write-time> <path>" for a file the launcher
// hashed or installed. A file whose size and last-write time still match is
// trusted without rereading it; anything else is hashed again. The cache is a
// performance hint only: a missing or malformed cache just forces hashing.

#include "PatchManifest.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>

namespace wydpatch {

struct CacheEntry {
    std::string sha256;
    std::uint64_t size = 0;
    std::uint64_t writeTime = 0;
};

// Keyed by FoldPath(path).
using PatchCache = std::unordered_map<std::string, CacheEntry>;

inline PatchCache ParseCache(std::string_view text)
{
    PatchCache cache;
    std::size_t position = 0;
    while (position < text.size()) {
        std::size_t end = text.find('\n', position);
        if (end == std::string_view::npos)
            end = text.size();
        std::string_view line = text.substr(position, end - position);
        position = end + 1;
        if (!line.empty() && line.back() == '\r')
            line.remove_suffix(1);
        const auto fields = SplitFields(line);
        CacheEntry entry;
        if (fields.size() != 4 || !IsSha256Hex(fields[0]) ||
            !ParseSize(fields[1], kMaxFileSize, entry.size) ||
            !ParseSize(fields[2], UINT64_MAX, entry.writeTime) ||
            !IsSafeRelativePath(fields[3]))
            continue;
        entry.sha256 = std::string(fields[0]);
        cache[FoldPath(fields[3])] = std::move(entry);
    }
    return cache;
}

inline std::string SerializeCacheLine(const std::string& path, const CacheEntry& entry)
{
    return entry.sha256 + ' ' + std::to_string(entry.size) + ' ' + std::to_string(entry.writeTime) + ' ' + path + '\n';
}

inline bool CacheMatches(const PatchCache& cache, const std::string& path, std::uint64_t size,
    std::uint64_t writeTime, const std::string& sha256)
{
    const auto found = cache.find(FoldPath(path));
    return found != cache.end() && found->second.size == size && found->second.writeTime == writeTime &&
        found->second.sha256 == sha256;
}

} // namespace wydpatch
