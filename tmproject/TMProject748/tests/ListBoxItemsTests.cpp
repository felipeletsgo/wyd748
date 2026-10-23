#include "../internal/ui/ListBoxItems.h"
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <windows.h>

// SListBox item ownership: each owned item is deleted exactly once, a
// pointer cannot be owned twice, and removal leaves no stale slot.
namespace
{
struct Item
{
    int deletes = 0;
};

void Destroy(Item* item)
{
    if (item)
        ++item->deletes;
}

std::string ReadListBoxSource()
{
    wchar_t executable[MAX_PATH]{};
    const auto length = GetModuleFileNameW(nullptr, executable, MAX_PATH);
    if (!length || length == MAX_PATH) return {};
    auto directory = std::filesystem::path(executable).parent_path();
    for (int depth = 0; depth < 8 && !directory.empty(); ++depth) {
        std::ifstream input(directory / "tmproject/TMProject748/internal/ui/controls/SListBox.cpp", std::ios::binary);
        if (input) return {std::istreambuf_iterator<char>(input), {}};
        directory = directory.parent_path();
    }
    return {};
}
}

int RunListBoxItemsTests(int& checks)
{
    int failures = 0;
    auto check = [&](bool condition, const char* message) {
        ++checks;
        if (!condition) {
            ++failures;
            std::printf("FAIL: %s\n", message);
        }
    };
    using list_box_items::AddResult;

    Item a, b, c, d;
    Item* items[4]{};
    int count = 0;
    check(list_box_items::Add(items, count, 3, &a, Destroy) == AddResult::Appended && count == 1, "first item appended");
    check(list_box_items::Add(items, count, 3, &b, Destroy) == AddResult::Appended && count == 2, "second item appended");
    check(list_box_items::Add(items, count, 3, &a, Destroy) == AddResult::Rejected && count == 2 && a.deletes == 0,
        "an already listed item is refused without deleting it");
    check(list_box_items::Add(items, count, 3, static_cast<Item*>(nullptr), Destroy) == AddResult::Rejected && count == 2,
        "a null item is refused");
    check(list_box_items::Add(items, count, 3, &c, Destroy) == AddResult::Appended && count == 3, "list fills to its limit");

    // A full list deletes its oldest item once and keeps the rest in order.
    check(list_box_items::Add(items, count, 3, &d, Destroy) == AddResult::Rotated, "full list rotates");
    check(count == 3 && items[0] == &b && items[1] == &c && items[2] == &d && items[3] == nullptr && a.deletes == 1,
        "rotation deletes the oldest item exactly once");
    check(list_box_items::Add(items, count, 3, &a, Destroy) == AddResult::Rotated && b.deletes == 1 && a.deletes == 1,
        "re-adding a removed pointer is a new owned entry");

    // Removal by index and by pointer clears the vacated slot.
    Item e, f, g;
    Item* removal[4]{};
    int removalCount = 0;
    list_box_items::Add(removal, removalCount, 4, &e, Destroy);
    list_box_items::Add(removal, removalCount, 4, &f, Destroy);
    list_box_items::Add(removal, removalCount, 4, &g, Destroy);
    check(list_box_items::RemoveAt(removal, removalCount, list_box_items::IndexOf(removal, removalCount, &f), Destroy),
        "remove by pointer");
    check(removalCount == 2 && removal[0] == &e && removal[1] == &g && removal[2] == nullptr && f.deletes == 1,
        "removal shifts later items and clears the vacated slot");
    check(!list_box_items::RemoveAt(removal, removalCount, list_box_items::IndexOf(removal, removalCount, &f), Destroy) &&
        f.deletes == 1, "removing an unlisted pointer deletes nothing");
    check(!list_box_items::RemoveAt(removal, removalCount, 2, Destroy) && !list_box_items::RemoveAt(removal, removalCount, -1, Destroy),
        "out-of-range removal is refused");
    list_box_items::Clear(removal, removalCount, Destroy);
    check(removalCount == 0 && removal[0] == nullptr && removal[1] == nullptr && e.deletes == 1 && g.deletes == 1,
        "clear deletes each owned item once and nulls every slot");

    // Capacity edge cases never write outside the array.
    Item h, i, j;
    Item* none[2]{};
    int noneCount = 0;
    check(list_box_items::Add(none, noneCount, 0, &h, Destroy) == AddResult::Discarded && noneCount == 0 && h.deletes == 1,
        "a list without capacity deletes the refused item");
    Item* clamp[2]{};
    int clampCount = 0;
    list_box_items::Add(clamp, clampCount, 5, &i, Destroy);
    list_box_items::Add(clamp, clampCount, 5, &j, Destroy);
    Item k;
    check(list_box_items::Add(clamp, clampCount, 5, &k, Destroy) == AddResult::Rotated && clampCount == 2 &&
        clamp[0] == &j && clamp[1] == &k && i.deletes == 1, "a limit larger than the array is clamped");
    // A capacity that shrank below the current count keeps the newest items.
    Item m1, m2, m3, m4;
    Item* shrink[4]{};
    int shrinkCount = 0;
    list_box_items::Add(shrink, shrinkCount, 4, &m1, Destroy);
    list_box_items::Add(shrink, shrinkCount, 4, &m2, Destroy);
    list_box_items::Add(shrink, shrinkCount, 4, &m3, Destroy);
    check(list_box_items::Add(shrink, shrinkCount, 2, &m4, Destroy) == AddResult::Rotated && shrinkCount == 2 &&
        shrink[0] == &m3 && shrink[1] == &m4 && shrink[2] == nullptr && m1.deletes == 1 && m2.deletes == 1,
        "shrunk capacity trims the oldest items once");

    const auto source = ReadListBoxSource();
    check(!source.empty(), "SListBox source located");
    check(source.find("list_box_items::Add(m_pItemList") != std::string::npos &&
        source.find("list_box_items::RemoveAt(m_pItemList") != std::string::npos &&
        source.find("list_box_items::Clear(m_pItemList") != std::string::npos &&
        source.find("SAFE_DELETE(m_pItemList") == std::string::npos,
        "SListBox owns items only through list_box_items");
    return failures;
}
