#pragma once

#include <cstddef>

namespace field_interaction
{
constexpr unsigned int AutoTradeFirstListingControl = 653;
constexpr unsigned int AutoTradePurchaseMessage = 646;

// EF_AMOUNT is a byte. Splitting must leave both stacks nonempty.
constexpr bool IsValidStackSplitQuantity(long long amount, int total)
{
    return total > 1 && total <= 255 && amount > 0 && amount < total;
}

// Validate a non-owning selection without dereferencing it: asynchronous
// inventory updates may have removed it from the live grid.
template <typename Item, std::size_t Count>
Item* FindOwnedItem(Item* const (&items)[Count], int activeCount, Item* selection)
{
    if (!selection || activeCount < 0 || static_cast<std::size_t>(activeCount) > Count)
        return nullptr;
    for (int i = 0; i < activeCount; ++i)
        if (items[i] == selection)
            return items[i];
    return nullptr;
}

// Keep the existing prompt ceiling; an occupied offer must have a positive price.
constexpr bool IsValidAutoTradePrice(long long price)
{
    return price > 0 && price < 2000000000LL;
}

// Inspect the complete wire array, including the last two native listings.
template <typename Item, std::size_t Count>
constexpr bool HasAutoTradeOffers(const Item (&items)[Count])
{
    for (const auto& item : items)
        if (item.sIndex > 0)
            return true;
    return false;
}

// Apply the sold-slot delta without relying on optional UI controls.
template <typename Item, typename Position, typename Price, std::size_t Count>
constexpr bool ClearAutoTradeOffer(Item (&items)[Count], Position (&positions)[Count],
    Price (&prices)[Count], int slot)
{
    if (slot < 0 || static_cast<std::size_t>(slot) >= Count)
        return false;
    items[slot] = {};
    positions[slot] = static_cast<Position>(-1);
    prices[slot] = 0;
    return true;
}

constexpr int AutoTradeSlotIndex(bool nativeHUD, unsigned int controlID)
{
    const unsigned int count = nativeHUD ? 12u : 10u;
    return controlID >= AutoTradeFirstListingControl &&
        controlID - AutoTradeFirstListingControl < count
        ? static_cast<int>(controlID - AutoTradeFirstListingControl) : -1;
}

// A confirmation belongs to the snapshot shown when the player selected it.
// -1 invalidates every purchase on replacement/close; a sale affects one slot.
constexpr bool ShouldCancelAutoTradePurchase(unsigned int message,
    unsigned int controlID, int invalidatedSlot)
{
    return message == AutoTradePurchaseMessage &&
        (invalidatedSlot == -1 || (invalidatedSlot >= 0 && invalidatedSlot < 12 &&
            AutoTradeSlotIndex(true, controlID) == invalidatedSlot));
}

// The native HUD has no 7.69 quick-slot widgets. Its Q/W/E/R/T handlers
// must receive the key even when the corresponding modern widget is absent.
constexpr int QuickSlotIndex(bool nativeHUD, char key)
{
    if (nativeHUD) return -1;
    switch (key)
    {
    case 'Q': case 'q': return 0;
    case 'W': case 'w': return 1;
    case 'E': case 'e': return 2;
    case 'R': case 'r': return 3;
    case 'T': case 't': return 4;
    default: return -1;
    }
}

// 0x3AE also acknowledges recall, logout and server selection. An ack alone
// is not permission to close the application: a local quit must be pending.
constexpr bool ShouldCloseOnDelayAck(unsigned int quitStartedAt)
{
    return quitStartedAt != 0;
}
}
