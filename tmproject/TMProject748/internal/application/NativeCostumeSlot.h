#pragma once

namespace costume_slot
{
// Costume slot contract (EXTENSAO_COORDENADA, server model/costume_slot.go).
// Native 7.48 equips costumes and familiars in slot 13 and the guild medal in
// slot 12. Medals are server-side state in WYD-Go, so costumes move to slot 12
// and slot 13 keeps the familiar; both can be worn together. A costume is an
// ItemList row with the costume body mesh 36 and the native slot-13 position.
inline constexpr int CostumeSlot = 12;
inline constexpr int FamiliarSlot = 13;
inline constexpr int CostumeMesh = 36;
inline constexpr int CostumeSlotMask = 1 << CostumeSlot;
inline constexpr int NativeCostumeSlotMask = 1 << FamiliarSlot;

constexpr bool IsCostume(int mesh, int position)
{
    return mesh == CostumeMesh && (position == NativeCostumeSlotMask || position == CostumeSlotMask);
}

// Applied once while ItemList.bin is loaded, matching the server catalog.
constexpr int RemapPosition(int mesh, int position)
{
    return mesh == CostumeMesh && position == NativeCostumeSlotMask ? CostumeSlotMask : position;
}
}
