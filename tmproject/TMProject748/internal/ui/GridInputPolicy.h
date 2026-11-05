#pragma once

#include <cmath>

// Pure decisions of SGridControl input. No scene, device or Win32 dependency.

namespace grid_cell_geometry
{
// Cell that contains `offset` (pixels from the grid origin) along one axis.
// SGridControl::FrameMove2 draws cell k at k * (extent / count); the hit test
// uses the same float cell size, so a pixel maps to the cell drawn under it
// (A26-02). Inside [0, extent) the result is in [0, count). Outside, it is the
// floor of the scaled offset and callers reject it with their range checks.
inline int CellAt(float offset, float extent, int count)
{
    if (count <= 0 || !(extent > 0.0f) || !std::isfinite(offset))
        return -1;
    const float cellSize = extent / static_cast<float>(count);
    const float scaled = std::floor(offset / cellSize);
    if (scaled < -1.0e6f || scaled > 1.0e6f)
        return scaled < 0.0f ? -1 : count;
    int cell = static_cast<int>(scaled);
    // Division can round across a drawn boundary; align with k * cellSize.
    if (static_cast<float>(cell) * cellSize > offset)
        --cell;
    else if (static_cast<float>(cell + 1) * cellSize <= offset)
        ++cell;
    if (offset >= 0.0f && offset < extent)
    {
        if (cell < 0)
            cell = 0;
        if (cell >= count)
            cell = count - 1;
    }
    return cell;
}

// Integer cell size kept for SwapItem's footprint guard (legacy contract).
inline int LegacyCellSize(float extent, int count)
{
    return count > 0 ? static_cast<int>(extent / static_cast<float>(count)) : 0;
}
}

namespace skill_belt_delete
{
constexpr int kSlotsPerPage = 10;
constexpr int kPages = 2;

// Shortcut index that Delete clears on the visible belt page. `localSlot` is
// the selected icon on that page, or -1 when the selection is not on it.
// Returns -1 when nothing must change (A26-03): no slot, and no packet.
constexpr int ShortcutIndex(int localSlot, bool secondPage)
{
    return localSlot < 0 || localSlot >= kSlotsPerPage
        ? -1
        : localSlot + (secondPage ? kSlotsPerPage : 0);
}
}
