#pragma once

#include "../../core/Structures.h"

namespace base_costume748 {
// Native 004fb34a..004fb6d2 runs after catalog selection and before the
// skin mesh is replaced. Other costumes retain their existing renderer.
inline bool ApplyLook(int costumeIndex, HUMAN_LOOKINFO& look, SANC_INFO& refinement)
{
    short faceMesh{};
    short bodyMesh{};
    switch (costumeIndex) {
    case 4153: faceMesh = 29; bodyMesh = 30; break;
    case 4154: faceMesh = 12; bodyMesh = 36; break;
    case 4155: faceMesh = 36; bodyMesh = 36; break;
    case 4156: faceMesh = 16; bodyMesh = 16; break;
    default: return false;
    }

    look.FaceMesh = faceMesh;
    look.FaceSkin = 0;
    look.HelmMesh = bodyMesh;
    look.CoatMesh = bodyMesh;
    look.PantsMesh = bodyMesh;
    look.GlovesMesh = bodyMesh;
    look.BootsMesh = bodyMesh;
    look.CoatSkin = 0;
    look.PantsSkin = 0;
    look.GlovesSkin = 0;
    look.BootsSkin = 0;
    // The native block does not write HelmSkin or either weapon's look.
    refinement.Sanc0 = refinement.Sanc1 = refinement.Sanc2 =
        refinement.Sanc3 = refinement.Sanc4 = refinement.Sanc5 = 0;
    refinement.Legend0 = refinement.Legend1 = refinement.Legend2 =
        refinement.Legend3 = refinement.Legend4 = refinement.Legend5 = 0;
    return true;
}
} // namespace base_costume748
