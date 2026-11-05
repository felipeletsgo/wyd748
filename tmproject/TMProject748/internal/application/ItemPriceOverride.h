#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

// Optional ItemPrice.bin display overrides: rows of two little-endian int32
// values {catalog index, price}, ended by the first zero index. The server
// owns prices. The file is validated as a whole before any catalog write.
namespace item_price_override
{
struct Entry
{
    int index;
    int price;
};

constexpr std::size_t kRowBytes = 2 * sizeof(std::int32_t);

// Returns the number of entries written to `entries` (capacity `maxRows`), or
// -1 to reject the file: a partial row, more than `maxRows` rows, or an index
// outside 1..catalogSize-1 before the terminator. An empty file has no entries.
inline int Parse(const unsigned char* bytes, std::size_t size, std::size_t maxRows,
    int catalogSize, Entry* entries)
{
    if (size == 0)
        return 0;
    if (!bytes || !entries || size % kRowBytes != 0 || size / kRowBytes > maxRows)
        return -1;

    int count = 0;
    for (std::size_t row = 0; row < size / kRowBytes; ++row)
    {
        std::int32_t index = 0;
        std::int32_t price = 0;
        std::memcpy(&index, bytes + row * kRowBytes, sizeof(index));
        std::memcpy(&price, bytes + row * kRowBytes + sizeof(index), sizeof(price));
        if (index == 0)
            break;
        if (index < 1 || index >= catalogSize)
            return -1;
        entries[count++] = Entry{index, price};
    }
    return count;
}
}
