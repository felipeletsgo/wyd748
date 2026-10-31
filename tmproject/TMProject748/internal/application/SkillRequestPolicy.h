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

// Samaritan (skill 3) is a self buff: the catalog marks it TargetType 1, but
// WYD-Go always casts it on the caster, never on players or monsters.
// WYD 7.48 Huntress skills by index: Enchant Frost (74), Backpack (85, opens
// Cargo anywhere) and Coin Armor (86, spends Trade x 100 gold).
constexpr int EnchantFrostSkill = 74;
constexpr int BackpackSkill = 85;
constexpr int CoinArmorSkill = 86;
constexpr int ClawWeaponType = 41;

// Owner rule: Enchant Frost cannot be cast with a claw. Hide in Shadow (75)
// and Another Soul (79) carry no weapon restriction.
constexpr bool SkillWeaponAllowed(int skillIndex, int weaponType)
{
    return skillIndex != EnchantFrostSkill || weaponType != ClawWeaponType;
}

constexpr bool IsSelfOnlySkill(int skillIndex)
{
    return skillIndex == 3;
}

constexpr bool AimsAtPrimaryTarget(int skillIndex)
{
    return skillIndex == 0 || skillIndex == 16 || skillIndex == 7 ||
        skillIndex == 17 || skillIndex == 23 || skillIndex == 35 ||
        skillIndex == 39 || skillIndex == 51 || skillIndex == 55 || skillIndex == 30;
}

// Rapid Hit is skill 95 in 7.48 (79 is Another_Soul, a buff). Both
// invocations check its primary-target range so a click reaches Range 6.
constexpr bool ChecksPrimaryTargetRange(int skillIndex, Invocation invocation)
{
    return AimsAtPrimaryTarget(skillIndex) || skillIndex == 95 ||
        (invocation == Invocation::Manual && skillIndex == 97);
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
