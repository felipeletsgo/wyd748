#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <sstream>
#include <string>

namespace asset_load
{
enum class LineResult { End, Blank, Record, Invalid };

// Each call consumes a complete physical line, including an overlong record.
inline LineResult ReadLine(FILE* file, char (&line)[512])
{
    if (!std::fgets(line, sizeof(line), file))
        return std::ferror(file) ? LineResult::Invalid : LineResult::End;
    if (!std::strchr(line, '\n') && !std::feof(file))
    {
        int ch;
        do { ch = std::fgetc(file); } while (ch != '\n' && ch != EOF);
        return LineResult::Invalid;
    }
    return line[std::strspn(line, " \t\r\n")] == '\0' ? LineResult::Blank : LineResult::Record;
}

inline bool ParseList(const char* line, int& id, std::string& path,
    int& count, int& parts, bool sound, bool bone, int capacity, std::size_t pathCapacity)
{
    if (!line) return false;
    std::istringstream input(line);
    int parsedID = -1, parsedCount = 1, parsedParts = 0;
    std::string parsedPath, extra;
    if (!(input >> parsedID)) return false;
    if (bone && !(input >> parsedCount >> parsedParts)) return false;
    if (!(input >> parsedPath)) return false;
    if (sound && !(input >> parsedCount)) return false;
    if (input >> extra) return false;
    if (parsedID < (sound ? 1 : 0) || parsedID >= capacity || parsedPath.size() >= pathCapacity)
        return false;
    if ((sound && (parsedCount < 1 || parsedCount > 512)) ||
        (bone && (parsedCount < 0 || parsedCount > 186 || parsedParts < 0 || parsedParts > 8))) return false;
    id = parsedID; count = parsedCount; parts = parsedParts; path = parsedPath;
    return true;
}

inline bool AnimationBytes(std::uint32_t ticks, std::uint32_t frames,
    std::uint32_t expectedFrames, std::size_t& bytes)
{
    if (!ticks || !frames || (expectedFrames && frames != expectedFrames)) return false;
    const std::uint64_t matrices = static_cast<std::uint64_t>(ticks) * frames;
    if (matrices > static_cast<std::uint64_t>((std::numeric_limits<int>::max)()) / 64) return false;
    bytes = static_cast<std::size_t>(matrices * 64);
    return true;
}

inline bool AnimationMapping(int category, int index)
{
    if (index < 0 || index == (std::numeric_limits<int>::max)()) return false;
    if (category > 5) return true; // No character mapping for these resources.
    const int number = index + 1;
    const int motion = number % 100 - 1;
    const int type = (category == 3 || category == 5) ? number / 100 + 59 : number / 100 - 1;
    return type >= 0 && type < 60 && motion >= 0 && motion < 56;
}

inline bool BoneSourcePath(const char* destination, char* source, std::size_t capacity)
{
    if (!destination || !source || !capacity) return false;
    source[0] = '\0';
    const std::size_t length = std::strlen(destination);
    if (length < 4 || length >= capacity || std::strcmp(destination + length - 4, ".dat"))
        return false;
    std::memcpy(source, destination, length + 1);
    std::memcpy(source + length - 3, "bon", 3);
    return true;
}
}
