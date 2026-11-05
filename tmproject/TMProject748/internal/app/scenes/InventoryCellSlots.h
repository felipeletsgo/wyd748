#pragma once

// Projection from a grid cell to a 7.48 wire slot.
//
// Only the Carry grid maps to Carry slots and only the Cargo grid maps to
// Cargo slots. Presentation grids (skill icons, trade offers, shop rows, mix
// staging) also use the default item type; projecting their cells with the
// Carry formula named unrelated Carry items in sale, use, swap and mix
// requests. A cell outside the native surface also yields no slot.
namespace inventory_cell_slots
{
constexpr int kColumns = 9;
constexpr int kCarryRows = 7;
constexpr int kVisibleCarrySlots = kColumns * kCarryRows;
constexpr int kUsableCargoSlots = 120;

inline int CarrySlot(bool isCarryGrid, int cellX, int cellY)
{
    if (!isCarryGrid || cellX < 0 || cellX >= kColumns || cellY < 0 || cellY >= kCarryRows)
        return -1;
    return cellX + kColumns * cellY;
}

inline int CargoSlot(bool isCargoGrid, int cellX, int cellY)
{
    if (!isCargoGrid || cellX < 0 || cellX >= kColumns || cellY < 0)
        return -1;
    if (cellY >= (kUsableCargoSlots + kColumns - 1) / kColumns)
        return -1;
    const int slot = cellX + kColumns * cellY;
    return slot < kUsableCargoSlots ? slot : -1;
}
}
