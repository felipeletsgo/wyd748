#pragma once

#include <cstddef>

// S->C ItemMix completion signal. The handler uses the envelope to close the active native panel.
// The result DWORD keeps its established offset.
constexpr auto MSG_CombineComplete_Opcode = 0x3A7;
constexpr std::size_t kCombineCompletePacketSize = 16;
constexpr std::size_t kCombineCompleteResultOffset = 12;

