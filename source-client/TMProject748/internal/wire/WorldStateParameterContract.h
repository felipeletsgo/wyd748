#pragma once

#include <cstddef>

// Respostas S->C de estado do mundo que compartilham MSG_STANDARDPARM no
// protocolo 7.48. Header.ID seleciona o receptor; o DWORD em +12 carrega saldo
// do Cargo ou tipo de remocao, conforme o opcode.
constexpr auto MSG_RemoveMob_Opcode = 0x165;
constexpr auto MSG_UpdateCargoGold_Opcode = 0x339;
constexpr std::size_t kWorldStateParameterPacketSize = 16;
constexpr std::size_t kWorldStateParameterValueOffset = 12;

