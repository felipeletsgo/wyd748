#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace party_message
{
template<std::size_t Rows, std::size_t Width, std::size_t Capacity>
bool Decode(const char* payload, std::size_t size, const char (&table)[Rows][Width], char (&text)[Capacity])
{
    static_assert(Capacity > 1 && Width > 0, "message buffers must not be empty");
    text[0] = '\0';
    if (!payload || size < 4 || payload[0] != 1) return false;
    std::int16_t code;
    std::memcpy(&code, payload + 2, sizeof(code));
    const int index = static_cast<int>(code) + 1000;
    if (index < 0 || static_cast<std::size_t>(index) >= Rows) return false;
    if (!table[index][0])
        std::snprintf(text, Capacity, "%d", static_cast<int>(code));
    else
    {
        std::size_t length = 0;
        while (length < Width && length < Capacity - 1 && table[index][length]) ++length;
        std::memcpy(text, table[index], length);
        text[length] = '\0';
    }
    return true;
}
}
