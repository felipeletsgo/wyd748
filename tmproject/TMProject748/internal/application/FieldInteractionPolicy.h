#pragma once

#include <cstddef>

namespace field_interaction
{
enum class PhysicalAttackAction { Cancel, Wait, Retry };

// Retrying an intention does not bypass the existing attack packet cooldown.
constexpr PhysicalAttackAction PhysicalAttackTick(bool alive, bool targetAlive,
    bool transitioning, bool combatControl, bool busy,
    unsigned int now, unsigned int lastRetry)
{
    return !alive || !targetAlive || transitioning || combatControl
        ? PhysicalAttackAction::Cancel
        : busy || now - lastRetry <= 200u
            ? PhysicalAttackAction::Wait : PhysicalAttackAction::Retry;
}

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

// The 7.48 auto-trade window has twelve listing grids (653..664).
constexpr int AutoTradeSlotIndex(unsigned int controlID)
{
    return controlID >= AutoTradeFirstListingControl &&
        controlID - AutoTradeFirstListingControl < 12u
        ? static_cast<int>(controlID - AutoTradeFirstListingControl) : -1;
}

// A confirmation belongs to the snapshot shown when the player selected it.
// -1 invalidates every purchase on replacement/close; a sale affects one slot.
constexpr bool ShouldCancelAutoTradePurchase(unsigned int message,
    unsigned int controlID, int invalidatedSlot)
{
    return message == AutoTradePurchaseMessage &&
        (invalidatedSlot == -1 || (invalidatedSlot >= 0 && invalidatedSlot < 12 &&
            AutoTradeSlotIndex(controlID) == invalidatedSlot));
}

// 0x3AE also acknowledges recall, logout and server selection. An ack alone
// is not permission to close the application: a local quit must be pending.
constexpr bool ShouldCloseOnDelayAck(unsigned int quitStartedAt)
{
    return quitStartedAt != 0;
}
}
