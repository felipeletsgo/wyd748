#pragma once
#include <cerrno>
#include <cctype>
#include <cstddef>
#include <cstdlib>

namespace quest_subject {
// Native FUN_0049ec64 reads the ten-byte level/color prefix and writes level-1.
// Reject malformed metadata before that write; valid resource indices retain
// their native meaning and later subject files still override earlier ones.
inline bool Apply(char* levels, std::size_t capacity, const char* row, char type) {
    if (!levels || !capacity || !row) return false;
    char prefix[11]{};
    for (std::size_t i = 0; i < 10 && row[i]; ++i) prefix[i] = row[i];
    char* end = nullptr;
    errno = 0;
    const long level = std::strtol(prefix, &end, 10);
    if (errno == ERANGE || end == prefix || level <= 0 ||
        static_cast<unsigned long>(level) > capacity ||
        !std::isspace(static_cast<unsigned char>(*end))) return false;
    while (std::isspace(static_cast<unsigned char>(*end))) ++end;
    if (!std::isxdigit(static_cast<unsigned char>(*end))) return false;
    char* colorEnd = nullptr;
    errno = 0;
    std::strtoul(end, &colorEnd, 16);
    if (errno == ERANGE || colorEnd == end) return false;
    while (std::isspace(static_cast<unsigned char>(*colorEnd))) ++colorEnd;
    if (*colorEnd) return false;
    levels[level - 1] = type;
    return true;
}
}
