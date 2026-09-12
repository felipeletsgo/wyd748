#pragma once

#include <cstddef>

// Snapshot S->C completo do estado de guerra. O peer WYD-Go sempre publica os
// tres DWORDs para que guilda inimiga, cla e aliada mudem de forma atomica.
constexpr auto MSG_WarInfo_Opcode = 0x3A8;
constexpr std::size_t kWarInfoPacketSize = 24;
constexpr std::size_t kWarInfoGuildOffset = 12;
constexpr std::size_t kWarInfoClanOffset = 16;
constexpr std::size_t kWarInfoAllyOffset = 20;
