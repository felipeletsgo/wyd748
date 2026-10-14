#pragma once

#include <array>
#include <utility>

namespace native_aylin
{
constexpr unsigned char RefinementEffect = 43;

constexpr bool IsRefinementOrTint(unsigned char effect)
{
    return effect == RefinementEffect || (effect >= 115 && effect <= 126);
}

// FUN_0054e75a, with the zero dye argument used by the Aylin preview caller.
template<class Item>
bool SetRefinement(Item& item, int level)
{
    if (level >= 26)
        return false;
    if (level < 0)
        level = 0;
    const auto value = static_cast<unsigned char>(level > 9 ? level + 220 : level);
    for (auto& effect : item.stEffect)
        if (effect.cEffect == RefinementEffect)
        {
            effect.cValue = value;
            return true;
        }
    for (auto& effect : item.stEffect)
        if (effect.cEffect >= 115 && effect.cEffect <= 126)
        {
            effect.cValue = value;
            return true;
        }
    for (auto& effect : item.stEffect)
        if (!effect.cEffect)
        {
            effect.cEffect = RefinementEffect;
            effect.cValue = value;
            return true;
        }
    return false;
}

// FUN_004110f5: presentation-only combinations; never mutate staged items.
template<class Item>
bool Build(const Item& first, const Item& second, int gem, int firstLevel,
    std::array<Item, 4>& results)
{
    results = {};
    if (first.sIndex <= 0 || second.sIndex <= 0 || gem < 2441 || gem > 2450)
        return false;
    Item normalized = first;
    if (!IsRefinementOrTint(normalized.stEffect[0].cEffect) &&
        normalized.stEffect[1].cEffect == RefinementEffect)
        std::swap(normalized.stEffect[0], normalized.stEffect[1]);
    if (!IsRefinementOrTint(normalized.stEffect[0].cEffect) &&
        normalized.stEffect[2].cEffect == RefinementEffect)
        std::swap(normalized.stEffect[0], normalized.stEffect[2]);

    results = { normalized, normalized, second, second };
    results[1].stEffect[1] = second.stEffect[1];
    results[2].sIndex = first.sIndex;
    results[3].sIndex = first.sIndex;
    results[3].stEffect[1] = normalized.stEffect[1];

    int level = firstLevel;
    if (level == 9)
        level = gem - 2431;
    else if (level == 10)
        level = gem - 2427;
    else if (level == 11)
        level = gem - 2423;
    for (auto& result : results)
        SetRefinement(result, level);
    return true;
}
}
