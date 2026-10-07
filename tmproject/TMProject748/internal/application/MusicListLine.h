#pragma once

#include <cstddef>
#include <cstring>

// One Music.txt line: a 17-column label, then the music path, as in
// "Thema            music\\login.mp3". The path ends at the line terminator;
// trailing tabs, spaces, CR and LF are removed. A line too short to reach the
// path column yields an empty path, so the caller keeps its default entry.
namespace music_list
{
constexpr std::size_t kPathColumn = 17;

inline bool ParseLine(const char* line, char* path, std::size_t pathSize)
{
    if (!line || !path || pathSize == 0)
        return false;
    path[0] = '\0';
    const std::size_t length = std::strlen(line);
    if (length <= kPathColumn)
        return false;

    std::size_t end = length;
    while (end > kPathColumn && (line[end - 1] == '\n' || line[end - 1] == '\r' ||
        line[end - 1] == '\t' || line[end - 1] == ' '))
        --end;
    const std::size_t count = end - kPathColumn;
    if (count == 0 || count >= pathSize)
        return false;
    std::memcpy(path, line + kPathColumn, count);
    path[count] = '\0';
    return true;
}
}
