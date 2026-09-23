#pragma once

#include <cstddef>

// Convite C<->S de desafio entre jogadores. Parm1 identifica o outro jogador
// e Parm2 carrega o modo (0..3 no convite, 4 na aceitacao do alvo).
constexpr auto MSG_PlayerChallenge_Opcode = 0x39F;
constexpr std::size_t kPlayerChallengePacketSize = 20;
constexpr std::size_t kPlayerChallengePlayerIdOffset = 12;
constexpr std::size_t kPlayerChallengeModeOffset = 16;
