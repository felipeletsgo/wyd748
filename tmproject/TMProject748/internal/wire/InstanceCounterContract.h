#pragma once

#include <cstddef>

// S->C counters emitted by WYD-Go during 7.48 instances.
constexpr auto MSG_InstanceTime_Opcode = 0x3A1;
constexpr auto MSG_InstanceMobs_Opcode = 0x3B0;
constexpr std::size_t kInstanceCounterPacketSize = 16;
constexpr std::size_t kInstanceCounterValueOffset = 12;
