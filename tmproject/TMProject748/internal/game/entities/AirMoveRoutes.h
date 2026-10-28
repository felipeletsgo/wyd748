#pragma once

struct AirMoveWaypoint
{
    int nX;
    int nY;
};

// Native 7.48 visual waypoints, shared by the scene and route regressions.
// Unspecified slots are the existing (0, 0) terminator.
inline constexpr AirMoveWaypoint g_pAirMoveRoute[5][10] =
{
    {{2112, 2051}},
    {{2372, 2099}},
    {{2485, 2097}, {2485, 1712}, {2220, 1714}},
    {{2356, 2112}, {2365, 2279}},
    {{2485, 2097}, {2485, 1712}, {2220, 1712}, {2220, 1593},
        {1982, 1593}, {1982, 1734}, {1826, 1771}}
};
