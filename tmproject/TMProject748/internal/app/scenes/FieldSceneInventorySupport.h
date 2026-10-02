#pragma once

// Both authoritative drop and merchant sale confirmations use this fixed
// resource-slot boundary. It does not change the persisted/wire Equip array.
bool WYD748_IsUnsupportedCompatEquipSlot(bool compat, int slot);
