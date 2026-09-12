#pragma once

#include <cstddef>

// Confirmacao S->C usada no relogin: o payload completo materializa MOB,
// slot, ClientID, clima, ShortSkill e os dois sidecars da ABI 7.48.
constexpr auto MSG_CNFCharacterLogin_Opcode = 0x114;
constexpr std::size_t kCharacterLoginConfirmPacketSize = 2104;
