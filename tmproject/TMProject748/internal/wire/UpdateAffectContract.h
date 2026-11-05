#pragma once

#include <cstddef>

// S->C affect snapshot consumed by the source client. The 7.48 handler reads sixteen eight-byte records.
// Level/Value use the coordinated STRUCT_AFFECT layout compiled here and the WYD-Go encoder.
constexpr auto MSG_UpdateAffect_Opcode = 0x3B9;
constexpr std::size_t kUpdateAffectPacketSize = 140;
constexpr std::size_t kUpdateAffectArrayOffset = 12;
constexpr std::size_t kUpdateAffectCount = 16;
constexpr std::size_t kUpdateAffectEntrySize = 8;
constexpr std::size_t kUpdateAffectTypeOffset = 0;
constexpr std::size_t kUpdateAffectLevelOffset = 1;
constexpr std::size_t kUpdateAffectValueOffset = 2;
constexpr std::size_t kUpdateAffectTimeOffset = 4;

