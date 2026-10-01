#pragma once

// Native 7.48 grid-type-3 display calculation (FUN_00418828).
// This is a quote, never authorization to mutate gold or inventory.
namespace native_sale_quote
{
    constexpr bool IsValidCatalogIndex(int itemIndex)
    {
        return itemIndex > 0 && itemIndex < 6500;
    }

    constexpr int Calculate(int itemIndex, int catalogPrice, int volatileAbility)
    {
        if (!IsValidCatalogIndex(itemIndex))
            return 0;

        // FILD/FMUL 0.25 followed by truncation, without float32 rounding.
        int price = catalogPrice / 4;
        if (price >= 5001 && price <= 10000)
            price = 2 * price / 3;
        else if (price > 10000)
            price /= 2;

        if (volatileAbility == 185)
            price = catalogPrice;
        if (itemIndex == 412)
            price = 800000;
        return price;
    }

    constexpr bool HasUnavailablePrice(int itemIndex, int catalogPrice)
    {
        return (itemIndex == 412 || itemIndex == 413) && catalogPrice == 0;
    }
}
