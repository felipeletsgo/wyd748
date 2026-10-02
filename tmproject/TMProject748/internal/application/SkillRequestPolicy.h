#pragma once

#include "../wire/AttackFrameContract.h"

namespace skill_request
{
enum class Invocation { Manual, Automatic };

// Catalog routing only; this is not server-side skill/target authorization.
constexpr bool UsesAreaRequest(int targetType)
{
    return targetType == 0 || targetType == 3 || targetType == 4 ||
        targetType == 5 || targetType == 6;
}

constexpr bool AimsAtPrimaryTarget(int skillIndex)
{
    return skillIndex == 0 || skillIndex == 16 || skillIndex == 7 ||
        skillIndex == 17 || skillIndex == 23 || skillIndex == 35 ||
        skillIndex == 39 || skillIndex == 51 || skillIndex == 55;
}

constexpr bool ChecksPrimaryTargetRange(int skillIndex, Invocation invocation)
{
    return AimsAtPrimaryTarget(skillIndex) || (invocation == Invocation::Manual
        ? skillIndex == 79 || skillIndex == 97 : skillIndex == 95);
}

constexpr int AreaRadius(int targetType)
{
    switch (targetType) {
    case 3: return 1;
    case 4: return 2;
    case 6: return 3;
    default: return -1;
    }
}

// Consume the legacy distance and hit-position results; do not replace either
// native geometry algorithm or move entity/zone queries ahead of this gate.
constexpr bool ReachesAreaTarget(int distance, int radius,
    int hitX, int hitY, int targetX, int targetY)
{
    return distance <= radius && hitX == targetX && hitY == targetY;
}

constexpr bool AreaTargetLimitReached(int targetType, int count, int maximumTargets)
{
    return (targetType == 3 && count > 7) || maximumTargets <= count ||
        count >= static_cast<int>(kAttackMultiTargetCapacity);
}
}
