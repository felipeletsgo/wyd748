#pragma once

#include <cstddef>

// Capacity rules of SEditableText (C-39). The text buffer and the display
// string of its GeomControl are fixed arrays; runtime callers may raise
// m_nMaxStringLen, so every write derives its bound from the arrays.
namespace editable_text_bounds
{
// Longest text the control keeps: the requested limit, at most the text
// storage minus its terminator, and at most the display string minus the
// caret and its terminator.
constexpr std::size_t MaxTextLength(std::size_t requested, std::size_t textCapacity,
    std::size_t displayCapacity)
{
    const std::size_t storageLimit = textCapacity > 0 ? textCapacity - 1 : 0;
    const std::size_t displayLimit = displayCapacity > 1 ? displayCapacity - 2 : 0;
    std::size_t limit = requested;
    if (limit > storageLimit)
        limit = storageLimit;
    if (limit > displayLimit)
        limit = displayLimit;
    return limit;
}

// The caret occupies display[length] and the terminator display[length + 1].
constexpr bool CaretFits(std::size_t length, std::size_t displayCapacity)
{
    return length + 2 <= displayCapacity;
}

// Mask characters that fit before the terminator of the display string.
constexpr std::size_t MaskLength(std::size_t length, std::size_t displayCapacity)
{
    return displayCapacity == 0 ? 0 : (length < displayCapacity ? length : displayCapacity - 1);
}

// Writes `length` mask characters and a terminator, truncated to the display.
// The visible mask equals the committed text length, as before.
template<std::size_t N>
std::size_t WriteMask(char (&display)[N], std::size_t length, char mask)
{
    const std::size_t count = MaskLength(length, N);
    for (std::size_t index = 0; index < count; ++index)
        display[index] = mask;
    display[count] = '\0';
    return count;
}

// Copies at most `length` bytes of `source` and terminates the destination.
template<std::size_t N>
std::size_t CopyPrefix(char (&destination)[N], const char* source, std::size_t length)
{
    std::size_t count = 0;
    if (source)
        while (count < length && count + 1 < N && source[count] != '\0')
        {
            destination[count] = source[count];
            ++count;
        }
    destination[count] = '\0';
    return count;
}
}
