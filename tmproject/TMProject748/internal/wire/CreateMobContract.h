#pragma once

#include <cstddef>

// Coordinated entity materialization used by TMProject748 and WYD-Go.
// Auto Trade retains the entity prefix and appends the shop title.
constexpr auto MSG_CreateMobTrade_Opcode = 0x363;
constexpr auto MSG_CreateMob_Opcode = 0x364;
constexpr std::size_t kCreateMobPacketSize = 328;
constexpr std::size_t kCreateMobTradePacketSize = 352;
constexpr std::size_t kCreateMobPositionOffset = 12;
constexpr std::size_t kCreateMobIdOffset = 16;
constexpr std::size_t kCreateMobNameOffset = 18;
constexpr std::size_t kCreateMobNameSize = 16;
constexpr std::size_t kCreateMobEquipOffset = 34;
constexpr std::size_t kCreateMobEquipCount = 18;
constexpr std::size_t kCreateMobAffectOffset = 70;
constexpr std::size_t kCreateMobAffectCount = 32;
constexpr std::size_t kCreateMobGuildOffset = 134;
constexpr std::size_t kCreateMobGuildLevelOffset = 136;
constexpr std::size_t kCreateMobScoreOffset = 140;
constexpr std::size_t kCreateMobScoreSize = 140;
constexpr std::size_t kCreateMobTypeOffset = 280;
constexpr std::size_t kCreateMobAncientOffset = 282;
constexpr std::size_t kCreateMobNickOffset = 300;
constexpr std::size_t kCreateMobNickSize = 26;
constexpr std::size_t kCreateMobServerOffset = 326;
constexpr std::size_t kCreateMobTabColorOffset = 327;
constexpr std::size_t kCreateMobTradeDescriptionOffset = 326;
constexpr std::size_t kCreateMobTradeDescriptionSize = 24;
constexpr std::size_t kCreateMobTradeServerOffset = 350;

