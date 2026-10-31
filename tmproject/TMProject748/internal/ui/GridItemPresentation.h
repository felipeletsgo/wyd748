#pragma once

#include <cmath>

namespace grid_item_presentation {
// FUN_0040fc3e centers all six native mix receptacles. The inherited
// 7.69 enum names for 16/18/20/22 do not describe their active 7.48 role.
constexpr bool IsNativeMixReceptacle(int gridType)
{
    return gridType >= 12 && gridType <= 22 && gridType % 2 == 0;
}

// Cape catalog offsets do not identify the intended blue/red/white icons in
// the active 7.48 model directory. Master capes reuse their elite icons.
// Correct grid presentation only; world capes, kingdom identity and the
// shared item catalog remain unchanged.
constexpr int IconTexture(int itemIndex, int catalogTexture)
{
    switch (itemIndex) {
    case 3191: return 25; // Elite Of Hekalotia: mt010128.wys, blue.
    case 3192: return 26; // Elite Of Akeronia: mt010129.wys, red.
    case 3193: return 27; // Elite Of Adventurer: mt010130.wys, white.
    case 3197: return 25; // Master Of Hekalotia reuses the blue elite icon.
    case 3198: return 26; // Master Of Akeronia reuses the red elite icon.
    case 3199: return 27; // Master Of Adventurer reuses the white elite icon.
    case 4006: return 17; // Apprentice Mantle: native mesh-27 grid maps 19 to 17.
    case 4148: return 8; // Mysterious Stone reuses Lactolerium 100% (3022).
    default: return catalogTexture;
    }
}

constexpr int IconMesh(int itemIndex, int catalogMesh)
{
    if (itemIndex == 4148)
        return 2874; // Lactolerium 100%: secretst.msa, not idealst.msa.
    return itemIndex >= 3197 && itemIndex <= 3199 ? 27 : catalogMesh;
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

// Equipment hand receptacles have a larger visual area than Carry's single
// wire cell. Fit that area without changing occupancy or authored mesh scale.
inline float WeaponMeshScale(float maxZ, float slotHeight, float uiScaleY)
{
    if (!std::isfinite(slotHeight) || !std::isfinite(uiScaleY) ||
        slotHeight <= 0.0f || uiScaleY <= 0.0f ||
        !std::isfinite(maxZ) || maxZ <= 0.0f)
        return 1.0f;
    const float target = slotHeight / (24.0f * uiScaleY) * 0.3f;
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
