#pragma once

#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstring>

// Pure decisions of the item tooltip (SGridTooltip.cpp). No scene dependency.

namespace tooltip_rows
{
// FieldScene2 has exactly the rows of the array. A row past it must not be
// written: the next scene member is m_pSkillHelpText (A27-03).
template<class T, std::size_t N>
T* At(T* (&rows)[N], int row)
{
    return row >= 0 && static_cast<std::size_t>(row) < N ? rows[row] : nullptr;
}
}

namespace nightmare_tooltip
{
constexpr unsigned int kRequestIntervalMs = 60000;

// The "nig" request refreshes the server clock. Send it when no reply is
// known or the last one is older than the interval (A27-01). The original
// test used `last ||`, which sent on every tooltip refresh after a reply.
constexpr bool ShouldRequestTime(unsigned int lastReplyTime, unsigned int now,
    unsigned int lastRequestTime = 0, bool hasRequest = false)
{
    return (lastReplyTime == 0 || now - lastReplyTime > kRequestIntervalMs) &&
        (!hasRequest || now - lastRequestTime > kRequestIntervalMs);
}
}

namespace refine_tooltip
{
// Percentage suffix of a refined item name. The success table has one entry
// per refine level; levels without an entry show no percentage (A27-02).
template<std::size_t N>
constexpr int SuccessPercent(int storedSuccess, int sanc, const int (&successRate)[N])
{
    return sanc >= 0 && static_cast<std::size_t>(sanc) + 1 < N
        ? storedSuccess * successRate[sanc + 1]
        : 0;
}
}

namespace tooltip_text
{
// Appends formatted text without using the destination as a printf source
// (C-35). Truncates at the buffer capacity and always terminates.
template<std::size_t N>
void Append(char (&buffer)[N], const char* format, ...)
{
    buffer[N - 1] = '\0';
    const std::size_t used = std::strlen(buffer);
    if (!format || used + 1 >= N)
        return;
    va_list arguments;
    va_start(arguments, format);
    const int written = std::vsnprintf(buffer + used, N - used, format, arguments);
    va_end(arguments);
    if (written < 0)
        buffer[used] = '\0';
}
}
