#pragma once

#include "MessageHeader.h"
#include <cstddef>
#include <cstdint>

// Aposta C->S e resultado S->C. Saldo, premio e jackpot continuam
// autoritativos no servidor; estes tipos apenas preservam o wire da UI.
constexpr auto MSG_DoJackpotBet_Opcode = 0x2BE;
constexpr auto MSG_ResultGamble_Opcode = 0x1BF;
struct MSG_DoJackpotBet
{
	MSG_STANDARD Header;
	std::int32_t GambleType;
	std::int32_t Bet;
};
struct MSG_ResultGamble
{
	MSG_STANDARD Header;
	std::uint8_t Result[5];
	std::uint8_t StopPosition[3];
	std::uint8_t Reserved[8];
	std::int32_t Prize;
	std::uint32_t Jackpot;
};
static_assert(sizeof(MSG_DoJackpotBet) == 20, "WYD 7.48 gamble request must be 20 bytes");
static_assert(sizeof(MSG_ResultGamble) == 36, "WYD 7.48 gamble result must be 36 bytes");
static_assert(offsetof(MSG_DoJackpotBet, GambleType) == 12, "gamble type offset changed");
static_assert(offsetof(MSG_DoJackpotBet, Bet) == 16, "gamble bet offset changed");
static_assert(offsetof(MSG_ResultGamble, Result) == 12, "gamble result offset changed");
static_assert(offsetof(MSG_ResultGamble, StopPosition) == 17, "gamble stops offset changed");
static_assert(offsetof(MSG_ResultGamble, Prize) == 28, "gamble prize offset changed");
static_assert(offsetof(MSG_ResultGamble, Jackpot) == 32, "gamble jackpot offset changed");
