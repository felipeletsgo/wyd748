#pragma once

#include <cstddef>

// A09-10: the 0x182 handler rejects items outside the loaded catalog, but the
// 0x114 login snapshot copies STRUCT_MOB equipment and carry slots unchecked.
// Later readers index g_pItemList with those values. This clears each slot
// whose index lies outside 0..catalogSize-1; zero is the wire empty slot.
namespace catalog_item_slots
{
template<class Item, std::size_t Count>
int ClearOutOfCatalog(Item (&items)[Count], int catalogSize)
{
    int cleared = 0;
    for (Item& item : items)
    {
        if (item.sIndex < 0 || item.sIndex >= catalogSize)
        {
            item = Item{};
            ++cleared;
        }
    }
    return cleared;
}
}
