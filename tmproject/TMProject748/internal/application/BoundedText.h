#pragma once

#include <cstddef>
#include <cstdio>
#include <cstring>

namespace bounded_text
{
    template<std::size_t N, class... Args>
    void Format(char (&destination)[N], const char* format, Args... args)
    {
        std::snprintf(destination, N, format, args...);
        destination[N - 1] = '\0';
    }

    // Wire fields pass their readable extent explicitly; C strings stop at NUL.
    template<std::size_t N>
    void Copy(char (&destination)[N], const char* source, std::size_t readable)
    {
        std::size_t length = 0;
        if (source)
            while (length < readable && length < N - 1 && source[length]) ++length;
        if (length) std::memmove(destination, source, length);
        destination[length] = '\0';
    }

    template<std::size_t Slots, std::size_t N, std::size_t M>
    bool CopyIndexedField(char (&destination)[Slots][N], int slot, const char (&source)[M])
    {
        if (slot < 0 || static_cast<std::size_t>(slot) >= Slots) return false;
        Copy(destination[slot], source, M);
        return true;
    }

    template<std::size_t N>
    void Prepare(char (&destination)[N], const char* source, std::size_t readable, bool group)
    {
        Copy(destination, source, readable);
        for (char& c : destination)
        {
            if (!c) break;
            if (c == '_') c = ' ';
        }
        if (destination[0] == ' ' && destination[1] == '\0') destination[0] = '\0';
        if (!group) return;

        char digits[N]{};
        std::size_t count = 0, spaces = 0;
        bool nonzero = false, prefixEnded = false, started = false;
        for (std::size_t i = 0; destination[i]; ++i)
        {
            const char c = destination[i];
            if (c == ' ')
            {
                ++spaces;
                if (started) prefixEnded = true;
            }
            else if (c >= '0' && c <= '9')
            {
                started = true;
                if (!prefixEnded && c != '0') nonzero = true;
                digits[count++] = c;
            }
            else return;
        }
        // Preserve the old zero/prefix behavior without converting a potentially
        // 255-digit value to int (atoi overflow is undefined).
        if (!nonzero) return;
        std::size_t out = 0;
        while (out < spaces && out < N - 1) destination[out++] = ' ';
        for (std::size_t i = 0; i < count; ++i)
        {
            const bool comma = i != 0 && (count - i) % 3 == 0;
            if (out + (comma ? 2 : 1) >= N) break;
            if (comma) destination[out++] = ',';
            destination[out++] = digits[i];
        }
        destination[out] = '\0';
    }
}
