#pragma once

#include <climits>

// Parts of the HUD skill-damage preview (BASE_GetSkillDamage). The server
// computes the applied damage; this only shapes the displayed estimate.
namespace skill_damage_preview
{
struct Caster
{
    int mobClass;
    int level;
    int special;
    int str;
    int intel;
    int attack;
    int weaponDamage;
};

// 7.48 names mapped to the TMSRVIN TK formula families. Huntress books
// retain their 7.48 tree ownership under the coordinated server contract.
inline int FormulaSkill(int mobClass, int skill) {
    if (mobClass != 0) return skill;
    switch (skill) {
    case 2: return 16;
    case 3: return 13;
    case 13: return 3;
    case 16: return 2;
    default: return skill;
    }
}

inline int Saturate(long long value)
{
    if (value > INT_MAX)
        return INT_MAX;
    if (value < INT_MIN)
        return INT_MIN;
    return static_cast<int>(value);
}

// Damage of an instance 1..5 skill before weather, amplification and finisher
// bonuses. A09-01: skill 97 (Conjure Cannon) is base + 15 * level for every
// class; the untransformed class switch lacked an else and overwrote it for
// classes 1..3. Other terms keep the existing client formula.
inline int InstanceBase(int skill, int base, bool transformed, const Caster& c)
{
    if (skill == 97)
        return Saturate(static_cast<long long>(base) + 15LL * c.level);
    const int levelPart = transformed ? c.level : c.level / 2;
    switch (c.mobClass)
    {
    case 0:
        if (skill / 8 == 1)
            return Saturate(3LL * c.weaponDamage + 3LL * c.str + levelPart + c.special + base);
        return Saturate(static_cast<long long>(c.weaponDamage) + c.intel / 4 + levelPart + c.special + base +
            c.intel / 40);
    case 1:
    case 2:
        return Saturate(static_cast<long long>(c.intel) / 30 + c.intel / 3 + levelPart + base +
            (transformed ? 2LL * c.special : c.special));
    case 3:
        if (skill == 95)
            return c.attack;
        return Saturate(3LL * c.weaponDamage + 3LL * c.str + c.level / 2 + c.special + base);
    default:
        return 0;
    }
}

// A09-01: the coordinated STRUCT_SCORE carries MagicAmp as uint32; the client
// truncated it to 8 bits. Large products saturate instead of wrapping.
inline int Amplify(int damage, unsigned int magicAmp)
{
    const long long factor = 4LL * static_cast<long long>(magicAmp) + 100;
    const long long magnitude = damage < 0 ? -static_cast<long long>(damage) : damage;
    if (magnitude != 0 && factor > LLONG_MAX / magnitude)
        return damage < 0 ? INT_MIN : INT_MAX;
    const long long scaled = static_cast<long long>(damage) * factor / 100;
    return Saturate(5 * scaled / 4);
}

// Instances outside 0..6 and 11 show twice the full MagicAmp.
inline int MagicAmpDamage(unsigned int magicAmp)
{
    return Saturate(2LL * static_cast<long long>(magicAmp));
}

inline int ManaCost(int base, int special, unsigned int saveMana)
{
    if (saveMana > 99) saveMana = 99;
    return Saturate(static_cast<long long>(base) * (100 + special / 2) / 100 * (100 - saveMana) / 100);
}

inline int CooldownSeconds(int skill, int catalogSeconds)
{
    // Existing coordinated Cancellation/Soul Limit policy; not native parity.
    // In the 7.48 book Cancellation is 100; 47 is Concentration.
    return skill == 100 || skill == 102 ? 1 : catalogSeconds;
}

inline int RefinedDamage(int value, int refinement)
{
    if (refinement <= 0) return value;
    if (refinement <= 10) return Saturate(static_cast<long long>(value) * (refinement + 10) / 10);
    static constexpr int percent[] = {220, 250, 280, 320, 370, 400};
    const int factor = refinement <= 16 ? percent[refinement - 11] : refinement - 10;
    return Saturate(static_cast<long long>(value) * factor / 100);
}

// Match World.equipmentDamage, including the established 7.48 learned-bit policy.
inline int EquipmentDamage(int other, int left, int right, int leftUnique,
    int rightUnique, int mobClass, unsigned int learned)
{
    int percent = 0;
    if (leftUnique && rightUnique) {
        percent = leftUnique == rightUnique ? 50 : 30;
        if (mobClass == 3 && (learned & 0x400)) percent += 10;
    }
    const int larger = left > right ? left : right;
    const int smaller = left > right ? right : left;
    return Saturate(static_cast<long long>(other) + larger + static_cast<long long>(smaller) * percent / 100);
}

// Pure projection of the integrated damage contract. Item and mount bonuses
// are supplied from the authoritative snapshot, not selection-screen data.
inline int Damage(int skill, int instance, int base, int affect, int weather,
    int face, Caster caster, unsigned int magicAmp, int baseMagic,
    unsigned int learned, int premiumMagicPercent)
{
    const int formula = FormulaSkill(caster.mobClass, skill);
    const int branch = formula % 24 / 8;
    int damage = 0;
    if (instance == 0) {
        switch (formula) {
        case 11: return affect + caster.special / 10;
        case 13: return affect + 3 * caster.special;
        case 41: return caster.special / 25 + 2;
        case 43: return affect + caster.special / 3 + 15;
        case 44: return 5 * (caster.special / 3 + 15);
        case 45: return affect + caster.special / 10;
        default: return 0;
        }
    }
    if (instance == 6) {
        damage = base + 3 * caster.special / 2;
        return skill == 29 && (learned & 0x80) ? 120 * damage / 100 : damage;
    }
    if (instance == 11) return base;
    if (instance < 1 || instance > 5) return MagicAmpDamage(magicAmp);
    if (skill == 95 && caster.mobClass == 3) return caster.attack;
    const bool magic = caster.mobClass != 3 && (caster.mobClass != 0 || formula / 8 != 1);
    damage = InstanceBase(formula, base, face % 10 > 5, caster);
    if (magic && skill != 97) damage = Saturate(static_cast<long long>(damage) + baseMagic);
    if (weather == 1) {
        if (instance == 2) damage = Saturate(90LL * damage / 100);
        if (instance == 5) damage = Saturate(130LL * damage / 100);
    } else if (weather == 2 && instance == 3) damage = Saturate(120LL * damage / 100);
    if (skill != 97) {
        damage = magic ? Amplify(damage, magicAmp) : Saturate(5LL * damage / 4);
        if (magic) damage = Saturate(static_cast<long long>(damage) * (100 + premiumMagicPercent) / 100);
    }
    if (caster.mobClass >= 0 && caster.mobClass < 4 && (learned & (1u << (8 * branch + 7)))) {
        static constexpr int multipliers[4][3] = {{115, 120, 115}, {110, 115, 115}, {110, 100, 100}, {110, 110, 120}};
        damage = Saturate(static_cast<long long>(damage) * multipliers[caster.mobClass][branch] / 100);
    }
    return damage;
}
}
