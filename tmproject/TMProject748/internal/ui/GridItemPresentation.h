#pragma once

#include <cmath>

namespace grid_item_presentation {
// The elite catalog offsets precede their intended blue/red/white textures
// in the active 7.48 model directory. Correct only grid presentation: world
// capes, kingdom identity and the shared item catalog remain unchanged.
constexpr int IconTexture(int itemIndex, int catalogTexture)
{
    switch (itemIndex) {
    case 3191: return 25; // Elite Of Hekalotia: mt010128.wys, blue.
    case 3192: return 26; // Elite Of Akeronia: mt010129.wys, red.
    case 3193: return 27; // Elite Of Adventurer: mt010130.wys, white.
    default: return catalogTexture;
    }
}

// FUN_0040E6AA/FUN_0040E817 apply the same fit at insertion, including
// equipment receptacles. Slot geometry changes centering, not mesh scale.
inline float MeshScale(float maxZ, int cellHeight)
{
    if (!std::isfinite(maxZ) || maxZ <= 0.0f || cellHeight <= 0)
        return 1.0f;
    const float target = static_cast<float>(cellHeight) * 0.3f;
    return maxZ > target ? target / maxZ : 1.0f;
}

inline const char* SplitCaption(const char* resource)
{
    if (resource) {
        for (const char* p = resource; *p; ++p)
            if (*p != ' ' && *p != '\t' && *p != '\r' && *p != '\n')
                return resource;
    }
    return "Enter the quantity to split.";
}

// The item visual and its text are separate draw-list nodes in 7.48.
// The caller supplies a final centre for both sprite and mesh footprints.
template<class Node, class Queue>
inline void QueueLabel(Node& label, float x, float y, int layer, Queue queue)
{
    if (!label.strString[0])
        return;
    label.nPosX = static_cast<decltype(label.nPosX)>(static_cast<int>(x));
    label.nPosY = static_cast<decltype(label.nPosY)>(static_cast<int>(y));
    label.nLayer = layer;
    queue(&label, layer);
}
}
