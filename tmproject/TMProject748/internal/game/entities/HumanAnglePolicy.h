#pragma once

namespace human_angle {

// Native float at 0x005a4290 (bits 0x40c90fdb).
constexpr float kFullTurn = 0x1.921fb6p+2f;

// Native human slot +0x48 preserves pitch for left-weapon ability 21 == 101.
// Ordinary mounted meshes add a full turn after reversing pitch.
constexpr float MeshPitch(int leftWeaponType, float pitch, bool mounted)
{
    if (leftWeaponType == 101)
        return pitch;
    return mounted ? -pitch + kFullTurn : -pitch;
}

// Native skin Render (0x004be3cb) excludes weapon 101 from the female Z reflection.
// Its pitch is already preserved by MeshPitch; reflecting it again reverses heading.
constexpr bool MirrorBodyZ(short meshType, int leftWeaponType, bool hasOwner)
{
    return hasOwner && meshType == 1 && leftWeaponType != 101;
}

} // namespace human_angle
