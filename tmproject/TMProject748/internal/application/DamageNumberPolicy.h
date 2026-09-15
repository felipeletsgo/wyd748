#pragma once

namespace damage_number
{
    inline int TextureSet(int type)
    {
        switch (type) {
        case 1: return 137;
        case 2: return 138;
        case 3: return 139;
        case 4: return 140;
        case 5: case 6: return 141;
        // Native 7.48 Blue_Number; 143 is a PGT button atlas, not magic digits.
        case 7: case 8: return 138;
        default: return -1;
        }
    }
}
