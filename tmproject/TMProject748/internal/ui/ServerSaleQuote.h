#pragma once
#include <algorithm>
#include <cstdint>

// Display the current Go merchant policy. The server owns the transaction.
// Keep this separate from the historical 7.48 response arithmetic.
namespace server_sale_quote {
inline int Calculate(int item, int catalogPrice, int characterClass,
    unsigned int learned, int trade, int tax, bool dionys)
{
    if (item <= 0 || item >= 6500 || catalogPrice < 0 || tax < 0 || tax > 100)
        return 0;
    std::int64_t price = catalogPrice / 4;
    if (dionys) {
        if (price > 10000) price /= 2;
        else if (price > 5000) price = price * 2 / 3;
    }
    if (characterClass == 3 && (learned & (1u << 9)) && price > 1000 &&
        item != 693 && item != 694 && item != 695) {
        const int bonus = (std::min)(26, (std::max)(0, trade) / 10 + 6) / 2;
        price = price * (50 + bonus) / 50;
    }
    return static_cast<int>(price - price * tax / 100);
}
}
