#pragma once

#include <cstddef>

// 7.48 C->S 0xAD9: MSG_STANDARDPARM2, rota em Parm1 e modo em Parm2.
constexpr unsigned int MSG_AirMove_Start_Opcode = 0xAD9;
constexpr int kAirMoveRouteCount = 5;
constexpr int kAirMoveStartMode = 1;
constexpr int kAirMoveEndMode = 2;
constexpr std::size_t kAirMovePacketSize = 20;
constexpr std::size_t kAirMoveRouteOffset = 12;
constexpr std::size_t kAirMoveModeOffset = 16;

constexpr bool IsValidAirMoveRouteIndex(int route)
{
    return route >= 0 && route < kAirMoveRouteCount;
}
