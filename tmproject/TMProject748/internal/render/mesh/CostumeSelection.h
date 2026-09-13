#pragma once
#include "CostumeTable.generated.h"

namespace costume748 {
inline constexpr int ImportedTag = 0x4000;

// The current server sends the full slot-13 ID. Never reconstruct a 4xxx ID
// from arbitrary low bits: an empty/unknown item must not acquire a costume.
inline const Item* FindItem(int index)
{
    for (const auto& item : Items)
        if (item.index == index) return &item;
    return nullptr;
}

inline int Select(int index, int skeleton)
{
    const auto* item = FindItem(index);
    if (!item) return 0;
    const int type = item->skin == -1 && (skeleton & 1)
        ? item->femaleType : item->maleType;
    // The validated manifest delegates these five base costumes to the
    // existing renderer; it deliberately supplies no replacement parts.
    if (type >= 1 && type <= 5) return 0;
    return ImportedTag | type;
}

inline const Renderer* FindRenderer(int taggedType)
{
    // Negative legacy types and untagged NPC skins retain their own renderer.
    if (taggedType < ImportedTag || taggedType >= ImportedTag * 2) return nullptr;
    for (const auto& renderer : Renderers)
        if (renderer.type == (taggedType & (ImportedTag - 1))) return &renderer;
    return nullptr;
}

enum class PartAction { Preserve, Replace, Skip };
inline PartAction ResolvePart(const Renderer* renderer, unsigned int index, const Part*& part)
{
    part = nullptr;
    if (!renderer || index >= 6) return PartAction::Preserve;
    const auto& candidate = renderer->parts[index];
    if (!candidate.mesh[0])
        return index == 0 ? PartAction::Preserve : PartAction::Skip;
    part = &candidate;
    return PartAction::Replace;
}
} // namespace costume748
