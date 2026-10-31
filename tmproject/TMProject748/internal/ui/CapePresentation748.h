#pragma once

namespace cape_presentation_748 {
constexpr int Refinement(int value)
{
    return value < 0 ? 0 : value > 15 ? 15 : value;
}

constexpr int PackedRefinement(unsigned short equipment)
{
    return Refinement(equipment >> 12);
}

// Citizen variants keep the emblem texture. The render path recolors the
// Apprentice Mantle variants 40..49 to preserve its green cloth.
constexpr int CitizenMantleSkin(int skin, int citizenship)
{
    if (citizenship < 1 || citizenship > 10 ||
        skin == 34 || skin == 35 || skin == 36)
        return skin;

    switch (skin)
    {
    case 19: return citizenship + 39;
    case 3: return citizenship + 49;
    case 2: return citizenship + 59;
    case 6: return citizenship + 69;
    case 1: return citizenship + 79;
    case 0: return citizenship + 89;
    case 7: return citizenship + 99;
    case 25: return citizenship + 109;
    case 24: return citizenship + 119;
    case 26: return citizenship + 129;
    case 28: return citizenship + 139;
    case 27: return citizenship + 149;
    case 29: return citizenship + 159;
    case 32: return citizenship + 169;
    case 31: return citizenship + 179;
    case 33: return citizenship + 189;
    default: return skin;
    }
}

// Inverse of native citizenship mapper FUN_00526cf4. UI set 65 contains
// base emblems 0..36, not the world cape's citizenship variants 40..199.
constexpr int BaseSkin(int skin)
{
    constexpr int bases[] = {19, 3, 2, 6, 1, 0, 7, 25, 24, 26, 28, 27, 29, 32, 31, 33};
    return skin >= 40 && skin <= 199 ? bases[(skin - 40) / 10] : skin;
}

constexpr int KingdomEmblem(int skin, int helmet)
{
    const int base = BaseSkin(skin);
    if (base < 0 || base > 36 || base == 19) return -1;
    return base == 2 && helmet >= 3503 && helmet <= 3506 ? 33 : base;
}
}
