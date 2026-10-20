#include "../internal/application/EntranceRunesPolicy.h"
#include <climits>
#include <cstdio>
#include <initializer_list>

int RunEntranceRunesTests(int& checks)
{
    int failures = 0;
    auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) { ++failures; std::printf("FAIL entrance runes: %s\n", name); }
    };
    using namespace entrance_runes;
    check(InHall(1046, 1690), "secret-room ticket tile is inside the hall");
    check(InHall(kHallMinX, kHallMinY) && InHall(kHallMaxX, kHallMaxY), "hall bounds are inclusive");
    check(!InHall(1038, 1690) && !InHall(1046, 1678) && !InHall(1046, 1702),
        "walls and outer field do not spawn runes");
    check(!InHall(808, 3676), "chamber rooms are not the entrance hall");
    for (int roll : {0, 1, 2, 3, 4, 7, 12345, INT_MAX, -1, INT_MIN})
    {
        const int texture = Texture(roll);
        check(texture >= 82 && texture <= 85, "only the colored pattern01..04 textures are used");
        const int x = RuneX(roll);
        const int y = RuneY(roll);
        check(x >= kRuneMinX && x <= kRuneMaxX && y >= kRuneMinY && y <= kRuneMaxY,
            "rune centers stay inside the hall margin");
        check(InHall(x - kGridCells, y - kGridCells) && InHall(x + kGridCells, y + kGridCells),
            "rune decal never reaches a wall");
        const unsigned int delay = NextDelayMs(roll);
        check(delay >= kMinDelayMs && delay < kMinDelayMs + kDelaySpreadMs, "spawn delay is bounded");
    }
    check(Texture(0) != Texture(1) && Texture(1) != Texture(2) && Texture(2) != Texture(3),
        "consecutive rolls select different rune colors");
    check(Due(1000, 1000) && Due(1001, 1000) && !Due(999, 1000), "schedule fires at its time");
    check(Due(5u, 0xFFFFFFF0u) && !Due(0xFFFFFFF0u, 5u), "schedule survives server-time wraparound");
    return failures;
}
