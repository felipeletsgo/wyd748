#pragma once

// Ordinary price bands shared by native 7.48 FUN_00487e23 (sale reply)
// and FUN_00418828 (grid quote). No item/ability override or server policy.
namespace native_sale_price
{
    constexpr int Calculate(int catalogPrice)
    {
        // FILD/FMUL 0.25 then truncation: keep the signed integer exact.
        // The two-thirds branch doubles at most 10000, avoiding overflow.
        int price = catalogPrice / 4;
        if (price >= 5001 && price <= 10000)
            price = 2 * price / 3;
        else if (price > 10000)
            price /= 2;
        return price;
    }
}
