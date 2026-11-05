#pragma once

// Local skill gates used before the client builds a skill request. The server
// validates every request; these decisions only avoid local reads and sends.
namespace skill_validity
{
// g_pSpell rows (MAX_SPELL_LIST); BasedefCombat.cpp asserts the equality.
constexpr int kSpellTableSize = 248;

// Shift counts outside 0..31 are undefined for a 32-bit mask; treat them as unset.
constexpr bool HasBit(unsigned int mask, int bit)
{
    return bit >= 0 && bit < 32 && ((mask >> bit) & 1u) != 0;
}

// Special skills 96..103 use LearnedSkill[0] bits 24..31 (native 7.48
// FUN_004431e4 skill panel; WYD-Go specialSkillBit uses index - 72).
constexpr bool IsSpecialSkillLearned(int index, unsigned int learned0)
{
    return index >= 96 && index < 104 && HasBit(learned0, index - 72);
}

// A09-05: the former gate accepted every index outside 0..95, including
// unlearned special skills and indexes beyond g_pSpell. Class skills 0..95
// still require the class tree and its learned bit. Indexes 104..247 keep the
// inherited newer-base acceptance; the server rejects what it does not own.
constexpr bool IsValidClassSkill(int index, int mobClass, unsigned int learned0)
{
    if (index < 0 || index >= kSpellTableSize)
        return false;
    if (index < 96)
        return index / 24 == mobClass && HasBit(learned0, index % 24);
    if (index < 104)
        return IsSpecialSkillLearned(index, learned0);
    return true;
}

// Unchanged decision of IsValidSkill with bounded shifts. Indexes 232..246 had
// shift counts of 32..44 (undefined); they now read as not learned.
constexpr bool IsValidSkill(int index, unsigned int learned0, unsigned int learned1)
{
    if (index >= 0 && index < 104)
        return index >= 96 ? IsSpecialSkillLearned(index, learned0) : HasBit(learned0, index % 24);
    if (index == 205 && HasBit(learned1, 5))
        return true;
    if (index == 233 && HasBit(learned1, 9))
        return true;
    if (index == 238 && HasBit(learned1, 2))
        return true;
    if (index < 200 || index >= 247 || index == 205 || index == 233 || index == 238)
        return false;
    return HasBit(learned1, 4 * ((index - 200) / 4));
}
}
