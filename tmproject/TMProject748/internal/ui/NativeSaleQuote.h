#pragma once

// Native 7.48 grid-type-3 display calculation (FUN_00418828).
// This is a quote, never authorization to mutate gold or inventory.
namespace native_sale_quote
{
    constexpr int VolatileEffect = 38;

    constexpr bool IsValidCatalogIndex(int itemIndex)
    {
        return itemIndex > 0 && itemIndex < 6500;
    }

    // Quote-specific type-38 path in FUN_0054cd07; other ability types are
    // deliberately left to their existing consumers. Storage bytes stay unsigned.
    template<class CatalogEffect, class InstanceEffect>
    constexpr int GetVolatileAbility(int itemIndex,
        const CatalogEffect (&catalogEffects)[12],
        const InstanceEffect (&instanceEffects)[3])
    {
        if (!IsValidCatalogIndex(itemIndex) || (itemIndex >= 3200 && itemIndex <= 3300))
            return 0;

        int ability = 0;
        for (const auto& effect : catalogEffects)
            if (effect.sEffect == VolatileEffect)
                ability += effect.sValue;

        // Mount fields are packed state, not ordinary instance effect pairs.
        if ((itemIndex >= 2330 && itemIndex <= 2389) ||
            (itemIndex >= 3980 && itemIndex <= 3999))
            return ability;

        for (const auto& effect : instanceEffects)
            if (effect.cEffect == VolatileEffect)
                ability += effect.cValue < 128 ? int(effect.cValue) : int(effect.cValue) - 256;
        // Type 38 skips refinement scaling; the native refinement callee has
        // no side effects, so its unused result need not be computed here.
        return ability;
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
