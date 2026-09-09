#pragma once

#include <cstddef>

// Sinal S->C de conclusao de ItemMix. O handler usa apenas o envelope e fecha
// o painel nativo ativo; o resultado DWORD permanece no offset convencional.
constexpr auto MSG_CombineComplete_Opcode = 0x3A7;
constexpr std::size_t kCombineCompletePacketSize = 16;
constexpr std::size_t kCombineCompleteResultOffset = 12;

