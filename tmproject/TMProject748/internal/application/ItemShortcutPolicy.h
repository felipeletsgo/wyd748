#pragma once

namespace item_shortcuts
{
enum class Action { None, ReservePress, Equipment, Warehouse };

// Win32 mouse messages and modifier bits, kept independent of Win32 headers.
constexpr Action Resolve(unsigned int message, unsigned int keys, bool inside,
    bool emptyCursor, bool storageGrid, bool equipmentGrid, bool warehouseOpen)
{
    if (!inside || !emptyCursor || (!storageGrid && !equipmentGrid))
        return Action::None;
    const bool ctrl = (keys & 8u) != 0;
    const bool shift = (keys & 4u) != 0;
    if (message == 0x204 && (ctrl || shift))
        return Action::ReservePress;
    if (ctrl && !shift && warehouseOpen && storageGrid && message == 0x201)
        return Action::ReservePress;
    if (ctrl && !shift && warehouseOpen && storageGrid && message == 0x202)
        return Action::Warehouse;
    if (message != 0x205)
        return Action::None;
    if (ctrl)
        return Action::Equipment;
    if (shift && warehouseOpen && storageGrid)
        return Action::Warehouse;
    return Action::None;
}

constexpr bool ValidSlot(int type, int slot)
{
    return slot >= 0 && ((type == 0 && slot > 0 && slot < 16) ||
        (type == 1 && slot < 63) || (type == 2 && slot < 120));
}

// Container dispatch uses parent-relative coordinates and ignores hidden trees.
template <typename Control>
bool VisibleHit(Control* control, int x, int y)
{
    if (!control)
        return false;
    const auto width = control->m_nWidth;
    const auto height = control->m_nHeight;
    int left = 0;
    int top = 0;
    for (auto* current = control; current;
        current = static_cast<Control*>(current->m_pTop))
    {
        if (!current->m_bVisible || current->m_cDeleted)
            return false;
        left += static_cast<int>(current->m_nPosX);
        top += static_cast<int>(current->m_nPosY);
    }
    return x >= left && y >= top && x < left + width && y < top + height;
}
}
