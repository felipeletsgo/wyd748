#pragma once

// Bone-animation index rules shared by TMSkinMesh frame, cut and swing code.
// Current mappings and assets satisfy them; the checks keep a future mapping
// or asset change from reading outside the animation tables.
namespace skin_animation
{
inline bool ValidSkeleton(int skeleton, int capacity)
{
    return skeleton >= 0 && skeleton < capacity;
}

inline bool ValidCut(int cut, unsigned int loadedCuts, unsigned int cutCapacity)
{
    return cut >= 0 && static_cast<unsigned int>(cut) < loadedCuts
        && static_cast<unsigned int>(cut) < cutCapacity;
}

// Skeleton 49 plays every cut without its last two ticks. Zero means the cut
// has no playable tick.
inline unsigned int PlayableTicks(int skeleton, unsigned int cutTicks)
{
    if (skeleton == 49)
        return cutTicks > 2 ? cutTicks - 2 : 0;
    return cutTicks;
}
}
