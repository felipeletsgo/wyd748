#include "../internal/application/ApplyCargoSlot.h"
#include <array>
#include <climits>
#include <cstdio>

// Without wire or Win32: exercise the mutation used by the cargo handler.
int RunCargoSlotTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) { ++failures; std::fprintf(stderr, "FAIL: %s\n", name); }
    };
    using Item = std::array<unsigned char, 8>;
    Item cargo[128]{};
    const Item item{0x34, 0x12, 1, 2, 3, 4, 5, 6};
    for (int position : {0, 119, 120, 127}) {
        Item before[128];
        std::memcpy(before, cargo, sizeof(cargo));
        check(ApplyCargoSlot(cargo, position, item), "valid cargo slot accepts a copy");
        bool unchanged = true;
        for (int i = 0; i < 128; ++i)
            if (i != position && cargo[i] != before[i]) unchanged = false;
        check(cargo[position] == item && unchanged,
            "eight bytes update only the selected slot, including reserved slots");
    }
    Item snapshot[128];
    std::memcpy(snapshot, cargo, sizeof(cargo));
    for (int position : {INT_MIN, -32768, -1, 128, 32767, INT_MAX})
        check(!ApplyCargoSlot(cargo, position, item), "invalid index does not write to cargo");
    check(std::memcmp(snapshot, cargo, sizeof(cargo)) == 0,
        "rejections preserve every cargo byte");
    check(ApplyCargoSlot(cargo, 127, cargo[127]) &&
        std::memcmp(snapshot, cargo, sizeof(cargo)) == 0,
        "reapplying the same slot is idempotent with an alias");
    return failures;
}
