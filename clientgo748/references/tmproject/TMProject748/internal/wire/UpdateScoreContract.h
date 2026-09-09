#pragma once

#include <cstddef>

// Extensao coordenada do snapshot de score usado exclusivamente pelo source
// client e WYD-Go. O layout e distinto do frame base de 92 bytes e da extensao
// historica XSC2 de 236 bytes observados na decompilacao estudada.
constexpr auto MSG_UpdateScore_Opcode = 0x336;
constexpr std::size_t kUpdateScorePacketSize = 232;
constexpr std::size_t kUpdateScoreScoreOffset = 12;
constexpr std::size_t kUpdateScoreAffectOffset = 152;
constexpr std::size_t kUpdateScoreAffectCount = 32;
constexpr std::size_t kUpdateScoreGuildOffset = 216;
constexpr std::size_t kUpdateScoreGuildLevelOffset = 218;
constexpr std::size_t kUpdateScoreReqHpOffset = 220;
constexpr std::size_t kUpdateScoreReqMpOffset = 224;
constexpr std::size_t kUpdateScoreLearnedSkillOffset = 228;

