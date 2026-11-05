#pragma once

#include <cstddef>

// Coordinated score snapshot for the source client and WYD-Go only.
// Its layout differs from the 92-byte base frame and historical 236-byte XSC2 extension in the studied decompilation.
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

