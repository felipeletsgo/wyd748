#pragma once

#include <cstddef>

// Native 7.48 Cargo gold requests and confirmations use a 16-byte envelope
// with one DWORD amount at +12. The server persists before confirming, then
// publishes authoritative 0x339 (Cargo) and 0x337 (character) snapshots.
constexpr auto MSG_Withdraw_Opcode = 0x387;
constexpr auto MSG_Deposit_Opcode = 0x388;
constexpr std::size_t kCargoGoldTransferPacketSize = 16;
constexpr std::size_t kCargoGoldTransferAmountOffset = 12;
