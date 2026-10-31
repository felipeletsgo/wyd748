#pragma once
#include <algorithm>
#include <cstdint>

namespace huntress_merchant_quote {
inline int PurchaseBase(int price, int characterClass, unsigned int learned, int trade)
{
    if (price <= 0 || characterClass != 3 || !(learned & (1u << 11))) return price;
    const int discount = (std::min)(26, 6 + (std::max)(0, trade) / 10);
    return static_cast<int>(static_cast<std::int64_t>(price) * (100 - discount) / 100);
}
}
