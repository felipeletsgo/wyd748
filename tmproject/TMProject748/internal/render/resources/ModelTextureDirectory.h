#pragma once

#include <cstddef>
#include <cstdio>
#include <cstring>
#include <string_view>

namespace model_texture_748 {
// Meshes request .wyt while the runtime may register a wrapped .wys file.
// Compare stems without rewriting the caller or reading empty table rows.
inline bool SameName(std::string_view requested, std::string_view registered)
{
    if (requested.empty() || registered.empty()) return false;
    if (requested == registered) return true;
    return requested.size() == registered.size() && requested.size() >= 5
        && requested[requested.size() - 4] == '.'
        && registered[registered.size() - 4] == '.'
        && requested.substr(0, requested.size() - 3)
            == registered.substr(0, registered.size() - 3);
}

template<class Row>
int Find(const char* name, const Row* rows, std::size_t capacity)
{
    if (!name || !rows) return -1;
    for (std::size_t i = 0; i < capacity; ++i) {
        const auto* end = static_cast<const char*>(
            std::memchr(rows[i].szFileName, '\0', sizeof(rows[i].szFileName)));
        if (end && SameName(name, {rows[i].szFileName,
                static_cast<std::size_t>(end - rows[i].szFileName)}))
            return static_cast<int>(i);
    }
    return -1;
}

// Native skins 40..199 request file numbers 41..200. The distributed table
// omits them; append a registry overlay preserving every existing texture ID.
// Preflight all dependencies/capacity before modifying any row.
template<class Row, class Exists>
bool RegisterCitizenMantles(Row* rows, std::size_t capacity, Exists exists)
{
    if (!rows) return false;
    static_assert(sizeof(rows[0].szFileName) >= sizeof("mesh\\mt0101200.wys"),
        "model texture name does not fit");
    std::size_t append = 0, missing = 0;
    for (std::size_t i = 0; i < capacity; ++i)
        if (rows[i].szFileName[0]) append = i + 1;
    char name[32]{};
    for (int file = 41; file <= 200; ++file) {
        std::snprintf(name, sizeof(name), "mesh\\mt0101%d.wys", file);
        if (!exists(name)) return false;
        if (Find(name, rows, capacity) < 0) ++missing;
    }
    if (missing > capacity - append) return false;
    for (int file = 41; file <= 200; ++file) {
        std::snprintf(name, sizeof(name), "mesh\\mt0101%d.wys", file);
        if (Find(name, rows, capacity) >= 0) continue;
        rows[append] = Row{};
        std::memcpy(rows[append].szFileName, name, std::strlen(name) + 1);
        rows[append].cAlpha = 'N';
        ++append;
    }
    return true;
}
// Append listed textures that the shipped table omits (the imported costume
// renderers need some rows the distributed MeshTextureList.bin never got).
// Existing IDs are preserved; a missing asset or insufficient capacity rejects
// the whole batch before any row changes.
template<class Row, class Entry, class Exists>
bool RegisterTextures(Row* rows, std::size_t capacity, const Entry* entries, std::size_t count, Exists exists)
{
    if (!rows || (!entries && count)) return false;
    std::size_t append = 0, missing = 0;
    for (std::size_t i = 0; i < capacity; ++i)
        if (rows[i].szFileName[0]) append = i + 1;
    for (std::size_t i = 0; i < count; ++i) {
        if (!entries[i].name || std::strlen(entries[i].name) >= sizeof(rows[0].szFileName)) return false;
        if (Find(entries[i].name, rows, capacity) >= 0) continue;
        if (!exists(entries[i].name)) return false;
        ++missing;
    }
    if (missing > capacity - append) return false;
    for (std::size_t i = 0; i < count; ++i) {
        if (Find(entries[i].name, rows, capacity) >= 0) continue;
        rows[append] = Row{};
        std::memcpy(rows[append].szFileName, entries[i].name, std::strlen(entries[i].name) + 1);
        rows[append].cAlpha = entries[i].alpha;
        ++append;
    }
    return true;
}
}
