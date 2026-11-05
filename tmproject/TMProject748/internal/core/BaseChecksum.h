#pragma once

namespace base_checksum
{
// Preserve the legacy signed-char arithmetic and the final else binding.
// This checksum detects asset changes; it does not authenticate assets.
inline int Sum(const char* bytes, int size)
{
    int sum = 0;
    for (int index = 0; index < size; ++index)
    {
        const int mod = index % 7;
        if (mod == 0)
            sum += bytes[index] / 2;
        if (mod == 1)
            sum += bytes[index] ^ 0xFF;
        if (mod == 2)
            sum += 3 * bytes[index];
        if (mod == 3)
            sum += 2 * bytes[index];
        if (mod == 4)
            sum -= bytes[index] / 7;
        if (mod == 5)
            sum -= bytes[index];
        else
            sum += bytes[index] / 3;
    }
    return sum;
}
}
