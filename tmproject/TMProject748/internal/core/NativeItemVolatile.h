#pragma once

// Fixed type-38 path in native 7.48 FUN_0054cd07. This read-only query
// neither authorizes item use nor changes the shared item/catalog storage ABI.
namespace native_item_volatile
{
    constexpr int Effect = 38;
    constexpr int CatalogLimit = 6500;

    constexpr bool IsValidCatalogIndex(int itemIndex)
    {
        return itemIndex > 0 && itemIndex < CatalogLimit;
    }

    template<class CatalogEffect, class InstanceEffect>
    constexpr int GetAbility(int itemIndex,
        const CatalogEffect (&catalogEffects)[12],
        const InstanceEffect (&instanceEffects)[3])
    {
        if (!IsValidCatalogIndex(itemIndex) || (itemIndex >= 3200 && itemIndex <= 3300))
            return 0;

        int ability = 0;
        for (const auto& effect : catalogEffects)
            if (effect.sEffect == Effect)
                ability += effect.sValue;

        // Mount fields are packed state, not ordinary instance effect pairs.
        if ((itemIndex >= 2330 && itemIndex <= 2389) ||
            (itemIndex >= 3980 && itemIndex <= 3999))
            return ability;

        for (const auto& effect : instanceEffects)
            if (effect.cEffect == Effect)
                ability += effect.cValue < 128 ? int(effect.cValue) : int(effect.cValue) - 256;
        // Type 38 skips refinement scaling; the native refinement callee has
        // no side effects, so its unused result need not be computed here.
        return ability;
    }
}
