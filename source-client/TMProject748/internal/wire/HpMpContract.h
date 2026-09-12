#pragma once

#include <cstddef>

// Extensao coordenada do source client com o WYD-Go. O frame publica os
// quatro recursos em uint32 e substitui, neste ecossistema, os envelopes
// historicos de 20/36 bytes ligados ao mesmo opcode.
constexpr auto MSG_SetHpMp_Opcode = 0x181;
constexpr std::size_t kHpMpPacketSize = 28;
constexpr std::size_t kHpMpCurrentHpOffset = 12;
constexpr std::size_t kHpMpCurrentMpOffset = 16;
constexpr std::size_t kHpMpMaximumHpOffset = 20;
constexpr std::size_t kHpMpMaximumMpOffset = 24;

