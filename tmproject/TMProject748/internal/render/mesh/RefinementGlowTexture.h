#pragma once

namespace refinement_glow
{
// Native FUN_004bbcb5 / FUN_004c3eec. Add the animation frame separately.
constexpr int TextureBase(int legend)
{
    switch (legend)
    {
    case 6: case 10: return 166; // Emerald: piercing.
    case 7: case 11: return 246; // Coral: EXP.
    case 8: case 12: return 260; // Garnet: absorption.
    default: return 153;        // Diamond, or the existing fallback.
    }
}
}
