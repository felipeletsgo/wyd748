#pragma once
#include <cstdint>

namespace cdata_integrity {
// WYD 7.48 FUN_00532390: unsigned arithmetic preserves the x86 checksum wrap.
inline std::uint32_t Checksum(const int (&data)[64][32]) {
    std::uint32_t sum = 0;
    for (unsigned row = 0; row < 64; ++row)
        for (unsigned column = 0; column < 32; ++column)
            sum += row * 8u + column * 16u + static_cast<std::uint32_t>(data[row][column]) * 4u;
    return sum;
}
constexpr std::uint32_t Expected = 0x231122b4u;
}
