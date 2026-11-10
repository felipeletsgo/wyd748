#pragma once

#include <cmath>

namespace grid_item_presentation {
// FUN_0040fc3e centers all six native mix receptacles. The inherited
// 7.69 enum names for 16/18/20/22 do not describe their active 7.48 role.
constexpr bool IsNativeMixReceptacle(int gridType)
{
    return gridType >= 12 && gridType <= 22 && gridType % 2 == 0;
}

// FUN_0040d13e adjusts the texture offsets for mesh-27 capes in the grid.
// Elite and Master share the approved gold-trimmed icons. Commander uses
// its equipped artwork because the native grid does not resolve both variants.
constexpr int IconTexture(int itemIndex, int catalogTexture)
{
    switch (itemIndex) {
    case 548:
    case 550: return 5; // Wanderer and Monster White.
    case 549: return 6; // Adventurer.
    case 1766: return 7;
    case 1768: return 9;
    case 1769: return 10;
    case 1771: return 12;
    case 572: return 13;
    case 574: return 15;
    case 4006: return 17; // Apprentice.
    case 4008: return 19; // Commander: mt010122.wys.
    case 4009: return 20; // Commander: mt010123.wys.
    case 3191:
    case 3194:
    case 3197: return 25; // Hekalotia, blue.
    case 3192:
    case 3195:
    case 3198: return 26; // Akelonia, red.
    case 3193:
    case 3196:
    case 3199: return 27; // Adventurer, white.
    case 4015: return 0; // Bersec uses the first material of dmantua.msa.
    case 4148: return 8; // Mysterious Stone reuses Lactolerium 100% (3022).
    default: return catalogTexture;
    }
}

constexpr int IconMesh(int itemIndex, int catalogMesh)
{
    if (itemIndex == 4015)
        return 2830; // Bersec Cape: dmantua.msa uses mt010124.wys.
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

inline const char* CaptionOrFallback(const char* resource, const char* fallback)
{
    if (resource) {
        for (const char* p = resource; *p; ++p)
            if (*p != ' ' && *p != '\t' && *p != '\r' && *p != '\n')
                return resource;
    }
    return fallback;
}

inline const char* SplitCaption(const char* resource)
{
    return CaptionOrFallback(resource, "Enter the quantity to split.");
}

inline const char* EntranceTicketCaption(const char* resource)
{
    return CaptionOrFallback(resource, "Use this Entrance Ticket to enter?");
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
