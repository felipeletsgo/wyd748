#pragma once

#include <cstddef>
#include <cstring>

// Coordinated tab-color v1: zero retains the existing gray nickname label.
// Unknown values from older/custom servers must never index past this table.
namespace tab_label
{
constexpr unsigned int kColors[]{
    0xFFCCCCCC, 0xFFFF7777, 0xFF77FF77, 0xFF7799FF, 0xFFFFFF66,
    0xFF66FFFF, 0xFFFF77FF, 0xFFFFAA55, 0xFFFFFFFF};
constexpr unsigned int Color(unsigned int index)
{
    return kColors[index < 9 ? index : 0];
}

inline bool IsCommand(const char* name)
{
    return name && (!std::strcmp(name, "tab") ||
        (std::strlen(name) == 4 && !std::strncmp(name, "tab", 3) &&
            name[3] >= '2' && name[3] <= '9'));
}

inline void CopyText(char (&destination)[26], const char* text)
{
    std::size_t length = 0;
    if (text)
        while (length < 25 && text[length]) ++length;
    if (length) std::memcpy(destination, text, length);
    std::memset(destination + length, 0, sizeof(destination) - length);
}
}
