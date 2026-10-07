#include "../internal/app/scenes/GroundNeighborPolicy.h"
#include <cstdio>

namespace {
// Models the scene's two terrain slots: at most one neighbor is attached, on
// the horizontal axis (1) or the vertical axis (2). Returns the loads issued
// while the player stays at (distX, distY) tiles from the two borders.
int LoadsAt(float distX, float distY, int frames, int& attachedAxis)
{
    int loads = 0;
    for (int frame = 0; frame < frames; ++frame) {
        ground_neighbor::Need needX{ distX, attachedAxis == 1 };
        ground_neighbor::Need needY{ distY, attachedAxis == 2 };
        if (ground_neighbor::ShouldAttach(needX, needY)) {
            attachedAxis = 1;
            ++loads;
            needX.attached = true;
            needY.attached = false;
        }
        if (ground_neighbor::ShouldAttach(needY, needX)) {
            attachedAxis = 2;
            ++loads;
        }
    }
    return loads;
}
}

int RunGroundNeighborPolicyTests(int& checks)
{
    int failures = 0;
    auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) { ++failures; std::printf("FAIL ground neighbor policy: %s\n", name); }
    };

    int axis = 0;
    check(LoadsAt(5.0f, 6.0f, 60, axis) == 1 && axis == 1,
        "standing in a corner loads the nearer neighbor once, not every frame");
    check(LoadsAt(7.0f, 5.0f, 60, axis) == 0 && axis == 1,
        "a small move toward the other border keeps the loaded neighbor");
    check(LoadsAt(9.0f, 4.0f, 60, axis) == 1 && axis == 2,
        "being clearly closer to the other border switches once");
    check(LoadsAt(6.0f, 4.0f, 60, axis) == 0 && axis == 2,
        "the switch back also needs the margin");

    axis = 0;
    check(LoadsAt(5.0f, -1.0f, 60, axis) == 1 && axis == 1, "a single border attaches its neighbor");
    axis = 2;
    check(LoadsAt(5.0f, -1.0f, 60, axis) == 1 && axis == 1,
        "a neighbor not serving a near border is replaced");
    axis = 0;
    check(LoadsAt(5.0f, 5.0f, 60, axis) == 1 && axis == 1, "a tie loads one neighbor");

    check(!ground_neighbor::ShouldAttach({ 0.0f, false }, { 4.0f, true }),
        "exact corner margin does not evict the attached neighbor");
    check(ground_neighbor::ShouldAttach({ 0.0f, false }, { 4.25f, true }),
        "crossing the corner margin permits replacement");
    for (int initial = 0; initial <= 2; ++initial) {
        for (int x = -1; x <= 20; ++x) {
            for (int y = -1; y <= 20; ++y) {
                axis = initial;
                const int loads = LoadsAt(static_cast<float>(x), static_cast<float>(y), 1, axis);
                check(loads <= 1, "one frame cannot attach both neighbor axes");
                const int stationaryLoads = LoadsAt(static_cast<float>(x), static_cast<float>(y), 120, axis);
                check(stationaryLoads == 0, "a stationary corner never reloads its neighbor");
            }
        }
    }
    return failures;
}
