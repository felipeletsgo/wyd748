#pragma once

namespace human_costume {
// Existing renderer membership only, not item validity or server refinement.
// Selection is independent of scene resources; TMHuman owns the six body writes.
constexpr bool UsesFixedBodyRefinement(int costume)
{
    switch (costume) {
    case 4150: case 4151: case 4169: case 4170:
    case 4171: case 4172: case 4173: case 4176:
    case 4177: case 4178: case 4179: case 4183:
        return true;
    default:
        return costume >= 4300 && costume <= 4420 &&
            costume != 4306 && costume != 4307 && costume != 4308 &&
            costume != 4319 && costume != 4375;
    }
}
} // namespace human_costume
