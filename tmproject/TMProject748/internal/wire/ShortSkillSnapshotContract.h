#pragma once

#include <cstddef>

// Bidirectional skill-belt snapshot. The client sends intent.
// The server returns twenty filtered shortcuts as authoritative state.
constexpr auto MSG_SetShortSkill_Opcode = 0x378;
constexpr std::size_t kShortSkillSnapshotPacketSize = 32;
constexpr std::size_t kShortSkillSnapshotSkillsOffset = 12;
constexpr std::size_t kShortSkillSnapshotSkillCount = 20;
