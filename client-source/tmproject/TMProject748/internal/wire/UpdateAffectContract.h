#pragma once

#include <cstddef>

// Snapshot S->C de affects consumido pelo source client. O handler 7.48
// percorre 16 registros de oito bytes; Level/Value seguem o layout coordenado
// da STRUCT_AFFECT compilada nesta arvore e do encoder WYD-Go.
constexpr auto MSG_UpdateAffect_Opcode = 0x3B9;
constexpr std::size_t kUpdateAffectPacketSize = 140;
constexpr std::size_t kUpdateAffectArrayOffset = 12;
constexpr std::size_t kUpdateAffectCount = 16;
constexpr std::size_t kUpdateAffectEntrySize = 8;
constexpr std::size_t kUpdateAffectTypeOffset = 0;
constexpr std::size_t kUpdateAffectLevelOffset = 1;
constexpr std::size_t kUpdateAffectValueOffset = 2;
constexpr std::size_t kUpdateAffectTimeOffset = 4;

