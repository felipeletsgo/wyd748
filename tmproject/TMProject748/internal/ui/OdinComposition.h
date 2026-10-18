#pragma once

namespace odin_composition {
// Existing server recipes share the two Locked Seals and Magic's Seal.
// Only the level-40 unlock uses these four ordered runes; cape composition
// retains its four Laktorerium Powders. The eighth wire slot stays empty.
template<class Item>
inline bool AcceptsSealRecipe(const Item (&items)[8])
{
    if (items[0].sIndex != 4127 || items[1].sIndex != 4127 ||
        items[2].sIndex != 5135 || items[7].sIndex != 0)
        return false;
    constexpr int runes[] = {5113, 5129, 5112, 5110};
    bool cape = true;
    bool unlock = true;
    for (int slot = 3; slot < 7; ++slot) {
        cape = cape && items[slot].sIndex == 413;
        unlock = unlock && items[slot].sIndex == runes[slot - 3];
    }
    return cape || unlock;
}
}
