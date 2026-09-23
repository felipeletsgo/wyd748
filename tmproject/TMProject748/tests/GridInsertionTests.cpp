#include "../internal/ui/GridInsertion.h"
#include <array>
#include <climits>
#include <cstdio>

// The same SGrid boundary has an observable consumer: rejection must not
// alter the list, occupancy, binding, or lifetime. DirectX rendering is not simulated.
int RunGridInsertionTests(int& checks)
{
    int failures = 0;
    auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) { ++failures; std::fprintf(stderr, "FAIL: %s\n", name); }
    };
    struct Item { int owner = -1; };
    Item* list[128]{};
    std::array<Item, 129> items{};
    int count = 0, calls = 0, occupied = 0;
    auto append = [&](Item* item) {
        return grid_insertion::Execute(list, count, item, [&] {
            ++calls;
            item->owner = count;
            list[count++] = item;
            ++occupied;
            return 1;
        });
    };
    check(append(nullptr) == 0 && calls == 0, "null item does not invoke the callback");
    for (int i = 0; i < 128; ++i)
        check(append(&items[i]) == 1 && list[i] == &items[i] && items[i].owner == i,
            "each valid slot receives ownership once");
    check(append(&items[128]) == 0 && count == 128 && calls == 128 &&
        occupied == 128 && items[128].owner == -1,
        "full list preserves occupancy, count, and caller ownership");
    for (int i = 0; i < 128; ++i)
        check(list[i] == &items[i], "rejection preserves every slot");
    for (int bad : {INT_MIN, -1, 129, INT_MAX}) {
        count = bad;
        check(append(&items[128]) == 0 && count == bad && calls == 128,
            "invalid count does not mutate state");
    }
    Item* small[2]{};
    check(grid_insertion::CanAppend(small, 1, &items[0]) &&
        !grid_insertion::CanAppend(small, 2, &items[0]),
        "capacity comes from the array type, not a fixed 128");
    int retries = 0;
    check(grid_insertion::Execute(small, 0, &items[0], [&] {
        ++retries; return 0;
    }) == 0 && retries == 1, "consumer failure propagates without retry");
    // Exercise the actual SGrid query across all positions and footprints
    // in a 9x7 grid against an independent cell enumeration.
    std::array<int, 63> cells{};
    cells[0] = 1;
    cells[31] = 1;
    cells[62] = 1;
    cells[10] = 2; // Legacy behavior blocks exactly 1, not every nonzero value.
    const auto original = cells;
    for (int y = -1; y <= 7; ++y)
        for (int x = -1; x <= 9; ++x)
            for (int height = 0; height <= 8; ++height)
                for (int width = 0; width <= 10; ++width) {
                    bool expected = x >= 0 && y >= 0 && width > 0 && height > 0 &&
                        x + width <= 9 && y + height <= 7;
                    if (expected)
                        for (int slot = 0; slot < 63; ++slot)
                            if (slot % 9 >= x && slot % 9 < x + width &&
                                slot / 9 >= y && slot / 9 < y + height && cells[slot] == 1)
                                expected = false;
                    check(grid_occupancy::CanPlace(cells.data(), 9, 7, x, y, width, height) == expected,
                        "query preserves occupancy and bounds for all small rectangles");
                }
    for (int bad : {INT_MIN, -1, 0, INT_MAX}) {
        check(!grid_occupancy::CanPlace(cells.data(), 9, 7, 0, 0, bad, 1), "invalid width rejected");
        check(!grid_occupancy::CanPlace(cells.data(), 9, 7, 0, 0, 1, bad), "invalid height rejected");
    }
    for (int bad : {INT_MIN, -1, INT_MAX}) {
        check(!grid_occupancy::CanPlace(cells.data(), 9, 7, bad, 0, 1, 1), "invalid x rejected");
        check(!grid_occupancy::CanPlace(cells.data(), 9, 7, 0, bad, 1, 1), "invalid y rejected");
    }
    check(!grid_occupancy::CanPlace(nullptr, 9, 7, 0, 0, 1, 1), "null occupancy rejected");
    check(!grid_occupancy::CanPlace(cells.data(), INT_MAX, 2, 0, 0, 1, 1), "invalid dimension product rejected");
    check(!grid_occupancy::CanPlace(cells.data(), 9, -1, 0, 0, 1, 1), "negative grid dimension rejected");
    check(!grid_occupancy::CanPlace(cells.data(), 0, 7, 0, 0, 1, 1), "empty grid rejected");
    check(cells == original, "query does not modify occupancy");
    // Sentinel-guarded buffer: cover equipment clipping, writes to occupied
    // cells, and symmetric removal without relying on the renderer.
    for (int y = 0; y < 7; ++y)
        for (int x = 0; x < 9; ++x)
            for (int height = 1; height <= 9; ++height)
                for (int width = 1; width <= 11; ++width) {
                    std::array<int, 65> buffer;
                    buffer.fill(77);
                    auto expected = buffer;
                    for (int slot = 0; slot < 63; ++slot)
                        if (slot % 9 >= x && slot % 9 < x + width &&
                            slot / 9 >= y && slot / 9 < y + height)
                            expected[slot + 1] = 1;
                    check(grid_occupancy::FillClipped(buffer.data() + 1, 9, 7, x, y, width, height, 1) &&
                        buffer == expected, "clipped write preserves sentinels and outside cells");
                    for (auto& cell : expected)
                        if (cell == 1) cell = 0;
                    check(grid_occupancy::FillClipped(buffer.data() + 1, 9, 7, x, y, width, height, 0) &&
                        buffer == expected, "removal clears exactly the inserted region");
                }
    const int invalidRects[][4] = {
        {-1, 0, 1, 1}, {0, -1, 1, 1}, {9, 0, 1, 1}, {0, 7, 1, 1},
        {INT_MIN, 0, 1, 1}, {INT_MAX, 0, 1, 1}, {0, INT_MIN, 1, 1},
        {0, INT_MAX, 1, 1}, {0, 0, 0, 1}, {0, 0, 1, 0},
        {0, 0, -1, 1}, {0, 0, 1, INT_MIN}, {1, 0, INT_MAX, 1}, {0, 1, 1, INT_MAX}
    };
    for (const auto& rect : invalidRects) {
        Item item;
        int localCount = 0;
        const auto before = cells;
        const int result = grid_insertion::Execute(small, localCount, &item, [&] {
            if (!grid_occupancy::FillClipped(cells.data(), 9, 7,
                rect[0], rect[1], rect[2], rect[3], 1)) return 0;
            item.owner = 0;
            ++localCount;
            return 1;
        });
        check(result == 0 && item.owner == -1 && localCount == 0 && cells == before,
            "invalid rectangle preserves memory, count, and ownership");
    }
    check(!grid_occupancy::FillClipped(nullptr, 9, 7, 0, 0, 1, 1, 1), "null write rejected");
    check(!grid_occupancy::FillClipped(cells.data(), INT_MAX, 2, 0, 0, 1, 1, 1), "invalid write dimension product");
    check(grid_occupancy::FillClipped(cells.data(), 9, 7, 0, 0, INT_MAX, INT_MAX, 1),
        "huge representable footprint is clipped without item-sized iteration");
    return failures;
}
