#pragma once

#include <cstddef>

constexpr auto MSG_CapsuleInfo_Opcode = 0xDC3;
constexpr std::size_t kCapsuleInfoPacketSize = 52;
constexpr std::size_t kCapsuleInfoIndexOffset = 12;
constexpr std::size_t kCapsuleInfoClassOffset = 16;
constexpr std::size_t kCapsuleInfoLevelOffset = 18;
constexpr std::size_t kCapsuleInfoStrengthOffset = 20;
constexpr std::size_t kCapsuleInfoIntelligenceOffset = 22;
constexpr std::size_t kCapsuleInfoDexterityOffset = 24;
constexpr std::size_t kCapsuleInfoConstitutionOffset = 26;
constexpr std::size_t kCapsuleInfoMasteryOffset = 28;
constexpr std::size_t kCapsuleInfoMasteryCount = 2;
constexpr std::size_t kCapsuleInfoSkillOffset = 32;
constexpr std::size_t kCapsuleInfoSkillCount = 9;
constexpr std::size_t kCapsuleInfoQuestOffset = 50;
