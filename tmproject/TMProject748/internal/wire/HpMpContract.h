#pragma once

#include <cstddef>

// Coordinated source-client/WYD-Go extension. Publish four uint32 resources.
// In this system, this replaces the historical 20/36-byte envelopes sharing this opcode.
constexpr auto MSG_SetHpMp_Opcode = 0x181;
constexpr std::size_t kHpMpPacketSize = 28;
constexpr std::size_t kHpMpCurrentHpOffset = 12;
constexpr std::size_t kHpMpCurrentMpOffset = 16;
constexpr std::size_t kHpMpMaximumHpOffset = 20;
constexpr std::size_t kHpMpMaximumMpOffset = 24;

