#pragma once

#include "../ui/ItemEffect.h"
#include <limits>

// Presentation of the existing server percentage contract, not a new score
// source. Ordinary adult mounts already contribute through canonical MagicAmp.
namespace premium_mount_magic {

template<class Definition>
int Percent(int index, const Definition& definition, int horsePercent)
{
    if (index <= 0 || (index >= 2330 && index < 2390))
        return 0;
    const auto ability = [&](int effect) {
        int value = 0;
        for (const auto& entry : definition.stEffect)
            if (entry.sEffect == effect)
                value += entry.sValue;
        return value;
    };
    if (ability(EF_NOTRADE) != 1)
        return 0;
    // These ten catalog horses encode duration in their name, not EF_DATE.
    if (index >= 3980 && index <= 3989)
        return horsePercent > 0 ? horsePercent : 0;
    const int days = ability(EF_DATE), hours = ability(EF_HOUR), minutes = ability(EF_MIN);
    if (days < 0 || hours < 0 || hours > 23 || minutes < 0 || minutes > 59
        || (days == 0 && hours == 0 && minutes == 0))
        return 0;
    const int percent = ability(EF_MAGIC);
    return percent > 0 ? percent : 0;
}

inline int Apply(int damage, int percent)
{
    if (damage <= 0 || percent <= 0)
        return damage;
    const auto projected = static_cast<long long>(damage) * (100LL + percent) / 100;
    return projected > (std::numeric_limits<int>::max)()
        ? (std::numeric_limits<int>::max)() : static_cast<int>(projected);
}

}
