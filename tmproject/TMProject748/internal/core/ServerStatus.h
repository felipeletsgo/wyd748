#pragma once

#include <cerrno>
#include <climits>
#include <cctype>
#include <cstddef>
#include <cstdlib>

// The status feed is advisory. Accept actual line endings as well as the
// literal "\n" separators expected by the previous scanner, without changing
// unread entries.
inline std::size_t ParseServerStatus(const char* text, int* values, std::size_t capacity)
{
    if (!text || !values)
        return 0;

    std::size_t count = 0;
    const char* cursor = text;
    while (count < capacity) {
        while (*cursor && std::isspace(static_cast<unsigned char>(*cursor)))
            ++cursor;
        if (cursor[0] == '\\' && cursor[1] == 'n') {
            cursor += 2;
            continue;
        }
        if (!*cursor)
            break;

        errno = 0;
        char* end = nullptr;
        const long value = std::strtol(cursor, &end, 10);
        if (end == cursor || errno == ERANGE || value < INT_MIN || value > INT_MAX)
            break;
        values[count++] = static_cast<int>(value);
        cursor = end;
        if (*cursor && !std::isspace(static_cast<unsigned char>(*cursor)) &&
            !(cursor[0] == '\\' && cursor[1] == 'n'))
            break;
    }
    return count;
}
