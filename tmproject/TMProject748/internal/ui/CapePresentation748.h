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
