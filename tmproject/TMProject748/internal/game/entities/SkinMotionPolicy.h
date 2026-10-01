#pragma once

#include "../../core/Enums.h"

namespace skin_motion {

// Skin 31 uses the second attack group. Change the enum value, not memory
// at an address derived from that value. All other motions remain unchanged.
constexpr ECHAR_MOTION Remap(int skinMeshType, ECHAR_MOTION motion)
{
    if (skinMeshType == 31 && motion >= ECHAR_MOTION::ECMOTION_ATTACK01 &&
        motion <= ECHAR_MOTION::ECMOTION_ATTACK03)
        return static_cast<ECHAR_MOTION>(static_cast<int>(motion) + 3);
    return motion;
}

} // namespace skin_motion
