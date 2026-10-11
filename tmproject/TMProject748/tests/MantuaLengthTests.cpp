#include "../internal/game/entities/MantuaLength.h"
#include <cstdio>

// Native FUN_00503a8c reads the cape length table at 0x005BB488 (8 x 20).
int RunMantuaLengthTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) { ++failures; std::printf("FAIL mantua length: %s\n", name); }
    };

    check(MantuaLength(0, 0) == 0.09f && MantuaLength(3, 19) == 0.08f, "base meshes read rows 0..3");
    check(MantuaLength(0, 41) == 0.035f && MantuaLength(1, 58) == 0.01f,
        "meshes 40+ read the second half, not the first rows again");
    check(MantuaLength(0, 21) == MantuaLength(1, 1), "meshes 20..39 cross rows like the native flat index");
    check(MantuaLength(3, 99) == MantuaLength(3, 0) && MantuaLength(0, -1) == MantuaLength(0, 0),
        "meshes outside the table use the class default");
    check(MantuaLength(7, 0) == MantuaLength(0, 0), "non-table skins never index past the table");
    return failures;
}
