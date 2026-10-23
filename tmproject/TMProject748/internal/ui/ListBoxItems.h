#pragma once

#include <cstddef>

// Ownership rules for SListBox's fixed item array. The list owns every item in
// [0, count); slots at or past count are always null, so no stale copy of an
// owned pointer survives a removal. An item can be owned only once: adding a
// pointer that is already listed is rejected without deleting it, because a
// second entry would later be deleted twice (the 2026-10-23 heap failure in
// the chat list freed an item that was already free).
namespace list_box_items
{
enum class AddResult
{
    Rejected,  // null or already listed; ownership unchanged
    Discarded, // the list has no capacity; the item was deleted
    Appended,  // stored at index count - 1 after count grew
    Rotated,   // the oldest item was deleted and the new one stored last
};

template <typename T, std::size_t N>
int IndexOf(T* const (&items)[N], int count, const T* item)
{
    for (int i = 0; i < count && i < static_cast<int>(N); ++i)
        if (items[i] == item)
            return i;
    return -1;
}

template <typename T, std::size_t N, typename Delete>
AddResult Add(T* (&items)[N], int& count, int limit, T* item, Delete destroy)
{
    if (item == nullptr || IndexOf(items, count, item) >= 0)
        return AddResult::Rejected;
    if (limit > static_cast<int>(N))
        limit = static_cast<int>(N);
    if (limit <= 0)
    {
        destroy(item);
        return AddResult::Discarded;
    }
    if (count >= limit)
    {
        // A list whose capacity shrank keeps only its newest limit - 1 items.
        const int drop = count - limit + 1;
        for (int i = 0; i < drop; ++i)
        {
            destroy(items[i]);
            items[i] = nullptr;
        }
        for (int i = drop; i < count; ++i)
        {
            items[i - drop] = items[i];
            items[i] = nullptr;
        }
        count = limit - 1;
        items[count++] = item;
        return AddResult::Rotated;
    }
    items[count++] = item;
    return AddResult::Appended;
}

template <typename T, std::size_t N, typename Delete>
bool RemoveAt(T* (&items)[N], int& count, int index, Delete destroy)
{
    if (index < 0 || index >= count || index >= static_cast<int>(N))
        return false;
    destroy(items[index]);
    for (int i = index + 1; i < count; ++i)
        items[i - 1] = items[i];
    items[--count] = nullptr;
    return true;
}

template <typename T, std::size_t N, typename Delete>
void Clear(T* (&items)[N], int& count, Delete destroy)
{
    for (int i = 0; i < count && i < static_cast<int>(N); ++i)
    {
        destroy(items[i]);
        items[i] = nullptr;
    }
    count = 0;
}
}
