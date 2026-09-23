#pragma once

#include <cstddef>
#include <cstring>

namespace wyd748::ui
{
// Replace the single %s in strdef[342] without treating asset or item bytes
// as a printf format. All inputs are fixed-width records from the 7.48 data.
inline bool FormatSellConfirmation(char* destination, std::size_t destinationCapacity,
    const char* pattern, std::size_t patternCapacity,
    const char* itemName, std::size_t itemNameCapacity)
{
    if (!destination || destinationCapacity == 0 || !pattern || !patternCapacity ||
        !itemName || !itemNameCapacity)
        return false;

    std::size_t patternLength = 0;
    while (patternLength < patternCapacity && pattern[patternLength])
        ++patternLength;
    if (patternLength == patternCapacity)
        return false;

    std::size_t marker = patternLength;
    for (std::size_t i = 0; i + 1 < patternLength; ++i)
    {
        if (pattern[i] == '%' && pattern[i + 1] == 's')
        {
            if (marker != patternLength)
                return false;
            marker = i;
            ++i;
        }
    }
    if (marker == patternLength)
        return false;

    std::size_t nameLength = 0;
    while (nameLength < itemNameCapacity && itemName[nameLength])
        ++nameLength;
    const std::size_t suffixLength = patternLength - marker - 2;
    if (marker >= destinationCapacity ||
        nameLength > destinationCapacity - marker - 1 ||
        suffixLength > destinationCapacity - marker - nameLength - 1)
        return false;

    std::memcpy(destination, pattern, marker);
    std::memcpy(destination + marker, itemName, nameLength);
    std::memcpy(destination + marker + nameLength, pattern + marker + 2, suffixLength);
    destination[marker + nameLength + suffixLength] = '\0';
    return true;
}
}
