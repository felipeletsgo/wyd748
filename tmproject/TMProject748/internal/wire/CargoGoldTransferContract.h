#pragma once

#include <cstddef>

// Pedido C->S e confirmacao S->C de transferencia de gold do Cargo. O WYD-Go
// devolve o mesmo envelope de um DWORD e, em seguida, publica os snapshots
// autoritativos 0x339 (Cargo) e 0x337 (personagem).
constexpr auto MSG_Withdraw_Opcode = 0x387;
constexpr auto MSG_Deposit_Opcode = 0x388;
constexpr std::size_t kCargoGoldTransferPacketSize = 16;
constexpr std::size_t kCargoGoldTransferAmountOffset = 12;
