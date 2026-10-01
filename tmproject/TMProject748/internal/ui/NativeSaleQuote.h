#pragma once

#include "../core/NativeItemVolatile.h"
#include "../core/NativeSalePrice.h"

// Native 7.48 grid-type-3 display calculation (FUN_00418828).
// This is a quote, never authorization to mutate gold or inventory.
namespace native_sale_quote
{
    constexpr int VolatileEffect = native_item_volatile::Effect;

    constexpr bool IsValidCatalogIndex(int itemIndex)
    {
        return native_item_volatile::IsValidCatalogIndex(itemIndex);
    }

    // Use the same fixed type-38 query as BASE_GetItemAbility.
    template<class CatalogEffect, class InstanceEffect>
    constexpr int GetVolatileAbility(int itemIndex,
        const CatalogEffect (&catalogEffects)[12],
        const InstanceEffect (&instanceEffects)[3])
    {
        return native_item_volatile::GetAbility(itemIndex, catalogEffects, instanceEffects);
    }

    constexpr int Calculate(int itemIndex, int catalogPrice, int volatileAbility)
    {
        if (!IsValidCatalogIndex(itemIndex))
            return 0;

        int price = native_sale_price::Calculate(catalogPrice);

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
