#include "../internal/ui/GridInsertion.h"
#include "../internal/ui/NativeSaleQuote.h"
#include "../internal/core/NativeSalePrice.h"
#include <array>
#include <climits>
#include <cstddef>
#include <cstdio>
#include <cstring>

// The same SGrid boundary has an observable consumer: rejection must not
// alter the list, occupancy, binding, or lifetime. DirectX rendering is not simulated.
int RunGridInsertionTests(int& checks)
{
    int failures = 0;
    auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) { ++failures; std::fprintf(stderr, "FAIL: %s\n", name); }
    };
    // The production MouseOver quote must preserve the native discontinuities
    // and override order; these fixtures do not simulate authoritative payment.
    const int quoteCases[][2] = {
        {0, 0}, {3, 0}, {4, 1}, {19999, 4999}, {20000, 5000},
        {20003, 5000}, {20004, 3334}, {39999, 6666}, {40000, 6666},
        {40003, 6666}, {40004, 5000}, {40007, 5000}, {40008, 5001},
        {16777223, 2097152}, {INT_MAX, 268435455}
    };
    for (const auto& fixture : quoteCases) {
        check(native_sale_price::Calculate(fixture[0]) == fixture[1],
            "legacy response preserves native price bands and full signed catalog precision");
        for (int itemIndex : {1, 413, 6499})
            check(native_sale_quote::Calculate(itemIndex, fixture[0], 0) == fixture[1],
                "ordinary quote and item 413 preserve native bands and integer precision");
        check(native_sale_quote::Calculate(1, fixture[0], 185) == fixture[0],
            "volatile ability 185 quotes the full catalog price after the bands");
        check(native_sale_quote::Calculate(413, fixture[0], 185) == fixture[0],
            "item 413 retains the volatile ability exception");
        for (int ability : {0, 184, 185, 186})
            check(native_sale_quote::Calculate(412, fixture[0], ability) == 800000,
                "item 412 quotes 800000 after every ability override");
    }
    check(native_sale_price::Calculate(1000000) == 125000 &&
        native_sale_quote::Calculate(412, 1000000, 185) == 800000,
        "response bands do not inherit the quote-only item 412 or ability 185 overrides");
    // Signed storage is not a reason to introduce a new negative-price policy.
    // These checks cover arithmetic only, not approval of negative sale credits.
    check(native_sale_price::Calculate(-3) == 0 &&
        native_sale_price::Calculate(-4) == -1 &&
        native_sale_price::Calculate(INT_MIN) == INT_MIN / 4,
        "shared price arithmetic preserves signed truncation without overflow or clamping");
    for (int itemIndex : {INT_MIN, -1, 0, 6500, INT_MAX}) {
        check(!native_sale_quote::IsValidCatalogIndex(itemIndex), "invalid quote index rejected");
        check(native_sale_quote::Calculate(itemIndex, INT_MAX, 185) == 0,
            "invalid quote index cannot receive the full-price override");
    }
    check(native_sale_quote::Calculate(1, 20004, 184) == 3334 &&
        native_sale_quote::Calculate(1, 20004, 186) == 3334,
        "neighboring ability values do not trigger the full-price override");
    for (int itemIndex : {1, 412, 413, 6499}) {
        check(native_sale_quote::HasUnavailablePrice(itemIndex, 0) ==
            (itemIndex == 412 || itemIndex == 413), "only zero-priced 412 and 413 replace the numeric quote");
        check(!native_sale_quote::HasUnavailablePrice(itemIndex, 4),
            "nonzero catalog prices retain the numeric quote");
    }
    // Match the borrowed production fields without pulling Windows/DirectX
    // dependencies into this pure contract test. The production template also
    // enforces the catalog/item array extents when SGrid is built.
    struct CatalogEffect { short sEffect, sValue; };
    struct InstanceEffect { unsigned char cEffect, cValue; };
    struct QuoteItem { short sIndex; InstanceEffect stEffect[3]; };
    static_assert(sizeof(CatalogEffect) == 4 && sizeof(QuoteItem) == 8 &&
        offsetof(QuoteItem, stEffect) == 2, "Native sale quote fixture ABI changed");
    CatalogEffect catalogEffects[12]{};
    QuoteItem quoteItem{1, {}};
    auto ability = [&] {
        return native_sale_quote::GetVolatileAbility(quoteItem.sIndex,
            catalogEffects, quoteItem.stEffect);
    };
    for (int slot = 0; slot < 3; ++slot) {
        for (int raw = 0; raw <= 255; ++raw) {
            quoteItem = QuoteItem{1, {}};
            quoteItem.stEffect[slot] = InstanceEffect{38, static_cast<unsigned char>(raw)};
            check(ability() == (raw < 128 ? raw : raw - 256),
                "each instance effect value is sign-extended over the entire byte domain");
            check(native_sale_quote::Calculate(1, 20004, ability()) == 3334,
                "an instance byte alone never produces the ability-185 full-price override");
        }
    }
    quoteItem = QuoteItem{1, {{38, 185}, {38, 185}, {38, 185}}};
    check(ability() == -213, "all three matching instance bytes accumulate as signed values");
    for (int slot = 0; slot < 12; ++slot) {
        for (short value : {static_cast<short>(SHRT_MIN), static_cast<short>(-1),
            static_cast<short>(185), static_cast<short>(SHRT_MAX)}) {
            for (auto& effect : catalogEffects) effect = CatalogEffect{};
            catalogEffects[slot] = CatalogEffect{38, value};
            quoteItem = QuoteItem{1, {}};
            check(ability() == value, "all twelve catalog slots retain signed word values");
        }
    }
    for (auto& effect : catalogEffects) effect = CatalogEffect{38, SHRT_MAX};
    quoteItem = QuoteItem{1, {{38, 127}, {38, 127}, {38, 127}}};
    check(ability() == 12 * SHRT_MAX + 381, "positive catalog and instance sums do not narrow");
    for (auto& effect : catalogEffects) effect = CatalogEffect{38, SHRT_MIN};
    quoteItem = QuoteItem{1, {{38, 128}, {38, 128}, {38, 128}}};
    check(ability() == 12 * SHRT_MIN - 384, "negative catalog and instance sums do not narrow");
    for (auto& effect : catalogEffects) effect = CatalogEffect{};
    catalogEffects[0] = CatalogEffect{38, 200};
    quoteItem = QuoteItem{1, {{38, 241}, {0, 0}, {0, 0}}};
    check(ability() == 185 && native_sale_quote::Calculate(1, 20004, ability()) == 20004,
        "catalog and signed instance sum equal to 185 enables the full-price override");
    const auto itemBeforeLookup = quoteItem;
    CatalogEffect catalogBeforeLookup[12];
    std::memcpy(catalogBeforeLookup, catalogEffects, sizeof(catalogEffects));
    (void)ability();
    check(std::memcmp(&itemBeforeLookup, &quoteItem, sizeof(quoteItem)) == 0 &&
        std::memcmp(catalogBeforeLookup, catalogEffects, sizeof(catalogEffects)) == 0,
        "lookup borrows inputs without mutating item or catalog storage");
    catalogEffects[0] = CatalogEffect{38, 185};
    catalogEffects[11] = CatalogEffect{38, 10};
    quoteItem = QuoteItem{1, {{38, 246}, {0, 0}, {0, 0}}};
    const int domainCases[][2] = {
        {1, 185}, {2329, 185}, {2330, 195}, {2389, 195}, {2390, 185},
        {3199, 185}, {3200, 0}, {3300, 0}, {3301, 185},
        {3979, 185}, {3980, 195}, {3999, 195}, {4000, 185}, {6499, 185}
    };
    for (const auto& fixture : domainCases) {
        quoteItem.sIndex = static_cast<short>(fixture[0]);
        check(ability() == fixture[1], "native special domains preserve their exact inclusive endpoints");
        check(native_sale_quote::Calculate(quoteItem.sIndex, 20004, ability()) ==
            (fixture[1] == 185 ? 20004 : 3334), "lookup domain controls only the ability quote exception");
    }
    for (int itemIndex : {INT_MIN, -1, 0, 6500, INT_MAX})
        check(native_sale_quote::GetVolatileAbility(itemIndex, catalogEffects, quoteItem.stEffect) == 0,
            "quote lookup rejects invalid catalog indices before consuming effects");
    catalogEffects[0] = CatalogEffect{37, 185};
    catalogEffects[11] = CatalogEffect{39, 185};
    for (int rawType = 0; rawType <= 255; ++rawType) {
        quoteItem = QuoteItem{1, {{static_cast<unsigned char>(rawType), 1}, {0, 0}, {0, 0}}};
        check(ability() == (rawType == 38 ? 1 : 0),
            "unrelated catalog and instance types cannot contribute to type 38");
    }
    catalogEffects[0] = CatalogEffect{38, 185};
    for (int raw = 0; raw <= 255; ++raw) {
        quoteItem = QuoteItem{1, {{43, static_cast<unsigned char>(raw)}, {0, 0}, {0, 0}}};
        check(ability() == 185, "type-38 catalog ability is independent of refinement bytes");
    }
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
