#pragma once

#include <cstddef>

// Complete S->C war-state snapshot. WYD-Go sends three DWORDs
// so enemy guild, clan and allied guild change atomically.
constexpr auto MSG_WarInfo_Opcode = 0x3A8;
constexpr std::size_t kWarInfoPacketSize = 24;
constexpr std::size_t kWarInfoGuildOffset = 12;
constexpr std::size_t kWarInfoClanOffset = 16;
constexpr std::size_t kWarInfoAllyOffset = 20;
