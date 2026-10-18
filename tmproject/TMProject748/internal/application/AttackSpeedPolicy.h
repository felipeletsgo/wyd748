#pragma once

namespace attack_speed {

// The existing 7.48 HUD calculation, shared with local attack prediction.
inline int Percent(unsigned int dex, unsigned int mastery, unsigned int learned,
    int itemBonus, int visualClass, int faceMesh, int speedUp, int speedDown,
    bool frozen)
{
    int classBonus = 0;
    if (visualClass == 26 && (faceMesh == 0 ||
        (faceMesh == 1 && (learned & 0x80000))))
        classBonus = 20;
    else if (visualClass == 33)
        classBonus = (learned & 0x200000) ? 40 : 20;
    else if (visualClass == 40)
        classBonus = 20;
    else if (visualClass == 63)
        classBonus = 30;
    return 100 + static_cast<int>(dex / 5) + itemBonus + classBonus
        + (static_cast<int>(mastery / 10) + 10) * (speedUp - speedDown)
        - (frozen ? 30 : 0);
}

// Project rule, not a claim about native probabilities. Retain packed bands
// through 200%; the unquantized HUD value makes 201..209% guaranteed too.
inline int DoubleChancePerMille(int percent, unsigned int attackRun)
{
    if (percent > 200)
        return 1000;
    const int chance = (static_cast<int>((attackRun >> 4) & 15) - 5) * 100;
    return chance < 0 ? 0 : chance > 1000 ? 1000 : chance;
}

}
