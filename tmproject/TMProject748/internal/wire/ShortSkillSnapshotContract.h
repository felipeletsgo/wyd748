#pragma once

#include <cstddef>

// Snapshot bidirecional da barra de skills. O client envia a intencao e o
// servidor devolve os vinte atalhos filtrados como estado autoritativo.
constexpr auto MSG_SetShortSkill_Opcode = 0x378;
constexpr std::size_t kShortSkillSnapshotPacketSize = 32;
constexpr std::size_t kShortSkillSnapshotSkillsOffset = 12;
constexpr std::size_t kShortSkillSnapshotSkillCount = 20;
