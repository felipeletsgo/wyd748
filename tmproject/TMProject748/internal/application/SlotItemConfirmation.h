#pragma once

#include <cstring>
#include <type_traits>

// A prompt owns a value snapshot, never a grid-item pointer. A slot update
// invalidates the confirmation even when the replacement uses the same icon.
template<class Item>
class SlotItemConfirmation
{
    static_assert(std::is_trivially_copyable<Item>::value, "Item snapshot must be a value");
public:
    void Capture(int slot, const Item* item)
    {
        slot_ = item ? slot : -1;
        item_ = item ? *item : Item{};
    }

    bool Take(int slot, const Item* item)
    {
        const bool matches = item && slot_ >= 0 && slot == slot_ &&
            std::memcmp(item, &item_, sizeof(Item)) == 0;
        slot_ = -1;
        return matches;
    }

private:
    int slot_ = -1;
    Item item_{};
};
