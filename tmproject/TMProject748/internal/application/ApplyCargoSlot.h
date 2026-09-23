#pragma once

#include <cstddef>
#include <cstring>
#include <type_traits>

// Locally update a server-confirmed slot without UI or protocol dependencies.
// The borrowed array determines storage capacity (128 in the client), not the
// player's usable limit (120). This function neither allocates nor retains references.
template <typename Item, std::size_t Capacity>
bool ApplyCargoSlot(Item (&cargo)[Capacity], int position, const Item& item)
{
    static_assert(std::is_trivially_copyable<Item>::value,
        "Cargo items must retain byte-copy semantics");
    if (position < 0 || static_cast<std::size_t>(position) >= Capacity)
        return false;

    // memmove preserves bytes even when source and destination are the same
    // slot. Rejection above leaves the entire array unchanged.
    std::memmove(&cargo[position], &item, sizeof(Item));
    return true;
}
