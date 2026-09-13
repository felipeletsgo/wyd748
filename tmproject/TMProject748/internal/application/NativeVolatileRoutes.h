#pragma once

namespace native_volatile
{
// UI intent only. The server resolves INDEX, effects, targets and consumption.
// Covered against the real volatiles/instances catalog by the server data tests.
enum class Route { Unknown, Direct, Target, Recall, Portal, Confirm, Capsule, CustomFirework, Interaction };
struct Entry { int first; int last; Route route; };

inline constexpr Entry ItemRoutes[] = {
    {3442, 3442, Route::CustomFirework},
    {3453, 3454, Route::Direct},
    {3455, 3455, Route::Interaction},
    {3909, 3910, Route::Direct},
    {3974, 3974, Route::Direct},
    {4003, 4005, Route::Direct},
    {4007, 4007, Route::Direct},
};
inline constexpr Entry CodeRoutes[] = {
    {0, 0, Route::Interaction},
    {1, 1, Route::Direct},
    {2, 3, Route::Interaction},
    {4, 5, Route::Target},
    {6, 8, Route::Direct},
    {9, 9, Route::Target},
    {10, 10, Route::Direct},
    {11, 11, Route::Recall},
    {12, 12, Route::Direct},
    {13, 13, Route::Recall},
    {15, 15, Route::Direct},
    {16, 16, Route::Target},
    {18, 19, Route::Direct},
    {20, 20, Route::Direct},
    {21, 28, Route::Direct},
    {30, 30, Route::Direct},
    {32, 36, Route::Direct},
    {41, 43, Route::Direct},
    {46, 48, Route::Direct},
    {51, 58, Route::Direct},
    {60, 67, Route::Direct},
    {70, 77, Route::Direct},
    {89, 89, Route::Direct},
    {90, 94, Route::Target},
    {131, 138, Route::Direct},
    {140, 140, Route::Direct},
    {161, 168, Route::Direct},
    {170, 172, Route::Direct},
    {173, 176, Route::Direct},
    {177, 177, Route::Interaction},
    {178, 178, Route::Direct},
    {179, 183, Route::Target},
    {184, 185, Route::Direct},
    {186, 186, Route::Target},
    {187, 187, Route::Confirm},
    {188, 189, Route::Direct},
    {190, 190, Route::Target},
    {191, 194, Route::Direct},
    {195, 195, Route::Portal},
    {196, 196, Route::Target},
    {197, 199, Route::Direct},
    {203, 203, Route::Direct},
    {206, 206, Route::Capsule},
    {207, 208, Route::Interaction},
    {211, 211, Route::Confirm},
    {212, 214, Route::Direct},
};

constexpr Route Resolve(int code, int itemIndex)
{
    for (const auto& entry : ItemRoutes)
        if (itemIndex >= entry.first && itemIndex <= entry.last) return entry.route;
    for (const auto& entry : CodeRoutes)
        if (code >= entry.first && code <= entry.last) return entry.route;
    return Route::Unknown;
}

constexpr bool AllowsRightClick(Route route)
{
    return route == Route::Direct || route == Route::Confirm || route == Route::Capsule;
}
}
