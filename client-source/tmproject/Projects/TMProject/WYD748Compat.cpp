#include "pch.h"

#include <cstddef>

#include "Basedef.h"

// These assertions protect the source client's side of the contract mirrored
// by internal/wire/score48.go. ExtendedScore remains server-authoritative.
static_assert(sizeof(STRUCT_SCORE) == 48, "STRUCT_SCORE wire ABI must remain 48 bytes");
static_assert(offsetof(STRUCT_SCORE, Level) == 0, "STRUCT_SCORE.Level offset changed");
static_assert(offsetof(STRUCT_SCORE, Ac) == 4, "STRUCT_SCORE.Ac offset changed");
static_assert(offsetof(STRUCT_SCORE, Damage) == 8, "STRUCT_SCORE.Damage offset changed");
static_assert(offsetof(STRUCT_SCORE, Reserved) == 12, "STRUCT_SCORE.Reserved offset changed");
static_assert(offsetof(STRUCT_SCORE, AttackRun) == 13, "STRUCT_SCORE.AttackRun offset changed");
static_assert(offsetof(STRUCT_SCORE, MaxHp) == 16, "STRUCT_SCORE.MaxHp offset changed");
static_assert(offsetof(STRUCT_SCORE, MaxMp) == 20, "STRUCT_SCORE.MaxMp offset changed");
static_assert(offsetof(STRUCT_SCORE, Hp) == 24, "STRUCT_SCORE.Hp offset changed");
static_assert(offsetof(STRUCT_SCORE, Mp) == 28, "STRUCT_SCORE.Mp offset changed");
static_assert(offsetof(STRUCT_SCORE, Str) == 32, "STRUCT_SCORE.Str offset changed");
static_assert(offsetof(STRUCT_SCORE, Special) == 40, "STRUCT_SCORE.Special offset changed");

// These packet assertions document the two first C->S authentication
// boundaries. They prevent a later source merge from silently moving the
// password, protocol marker or character-login proof again.
static_assert(sizeof(MSG_AccountLogin) == 116, "7.48 login request must remain 116 bytes");
static_assert(offsetof(MSG_AccountLogin, DBNeedSave) == 44, "source protocol marker offset changed");
static_assert(sizeof(MSG_CharacterLogin) == 36, "7.48 character login must remain 36 bytes");
static_assert(offsetof(MSG_CharacterLogin, Force) == 16, "7.48 character login Force offset changed");
static_assert(offsetof(MSG_CharacterLogin, SecretCode) == 20, "7.48 character login proof offset changed");
// These frequently emitted C->S packets previously retained newer-version
// padding and were rejected before their handlers could run.
static_assert(sizeof(MSG_DeleteCharacter) == 44, "7.48 delete-character request must remain 44 bytes");
static_assert(sizeof(MSG_NewCharacter) == 36, "7.48 create-character request must remain 36 bytes");
static_assert(sizeof(MSG_STANDARDPARM) == 16, "7.48 one-parameter packet must remain 16 bytes");
static_assert(sizeof(MSG_STANDARDPARM2) == 20, "7.48 two-parameter packet must remain 20 bytes");
static_assert(sizeof(MSG_STANDARDPARM3) == 24, "7.48 three-parameter packet must remain 24 bytes");
static_assert(sizeof(MSG_SwapItem) == 20, "7.48 swap request must remain 20 bytes");
static_assert(sizeof(MSG_REQShopList) == 16, "7.48 shop request must remain 16 bytes");
static_assert(sizeof(MSG_MessageChat) == 140, "7.48 outbound chat must remain 140 bytes");
static_assert(sizeof(MSG_MessageWhisper) == 128, "7.48 whisper must remain 128 bytes");
static_assert(sizeof(MSG_MessagePanel) == 108, "7.48 message panel must remain 108 bytes");
// Combat and small interaction requests are especially sensitive to implicit
// MSVC padding, so lock every size validated by WYD-Go's deny-by-default edge.
static_assert(sizeof(STRUCT_DAM) == 4, "7.48 damage pair must remain four bytes");
static_assert(sizeof(MSG_AttackOne) == 48, "7.48 AttackOne must remain 48 bytes");
static_assert(sizeof(MSG_AttackTwo) == 52, "7.48 AttackTwo must remain 52 bytes");
static_assert(sizeof(MSG_Attack) == 96, "7.48 AttackMulti must remain 96 bytes");
static_assert(sizeof(MSG_CNFParty2) == 32, "7.48 party confirmation must remain 32 bytes");
static_assert(sizeof(MSG_REQMobByID) == 16, "7.48 mob request must remain 16 bytes");
// Lock the remaining high-frequency gameplay requests to the exact sizes in
// internal/game/security.go. This turns future source merges into compile-time
// failures instead of silent disconnects at the server edge.
static_assert(sizeof(MSG_Action) == 52, "7.48 movement/action must remain 52 bytes");
// Size alone did not catch the imported 7.59 member order.  Lock every
// movement field consumed by WYD-Go so future merges fail at compile time.
static_assert(offsetof(MSG_Action, PosX) == 12, "7.48 movement PosX offset changed");
static_assert(offsetof(MSG_Action, Speed) == 16, "7.48 movement Speed offset changed");
static_assert(offsetof(MSG_Action, Effect) == 20, "7.48 movement Effect offset changed");
static_assert(offsetof(MSG_Action, TargetX) == 24, "7.48 movement TargetX offset changed");
static_assert(offsetof(MSG_Action, Route) == 28, "7.48 movement Route offset changed");
static_assert(sizeof(MSG_Motion) == 20, "7.48 motion must remain 20 bytes");
static_assert(sizeof(MSG_MOVESTOP) == 36, "7.48 stop request must remain 36 bytes");
static_assert(sizeof(MSG_UseItem) == 36, "7.48 use-item request must remain 36 bytes");
static_assert(sizeof(MSG_UseItem2) == 52, "7.48 premium-firework request must remain 52 bytes");
static_assert(sizeof(MSG_Buy) == 24, "7.48 buy request must remain 24 bytes");
static_assert(sizeof(MSG_Sell) == 20, "7.48 sell request must remain 20 bytes");
static_assert(sizeof(MSG_ApplyBonus) == 20, "7.48 apply-bonus request must remain 20 bytes");
static_assert(sizeof(MSG_REQParty) == 44, "7.48 party request must remain 44 bytes");
static_assert(sizeof(MSG_Trade) == 156, "7.48 trade request must remain 156 bytes");
static_assert(sizeof(MSG_AutoTrade) == 196, "7.48 auto-trade request must remain 196 bytes");
static_assert(sizeof(MSG_CombineItem) == 84, "7.48 combine request must remain 84 bytes");
static_assert(sizeof(MSG_DropItem) == 32, "7.48 drop request must remain 32 bytes");
static_assert(sizeof(MSG_GetItem) == 28, "7.48 pickup request must remain 28 bytes");
static_assert(sizeof(MSG_UpdateItem) == 20, "7.48 item-update request must remain 20 bytes");
static_assert(sizeof(MSG_SetShortSkill) == 32, "7.48 short-skill request must remain 32 bytes");
static_assert(sizeof(MSG_ReqBuy) == 36, "7.48 auto-trade purchase request must remain 36 bytes");

// The source-client response family deliberately keeps the imported 48-byte
// score and therefore cannot share the stock binary's compact S->C sizes.
// These values are mirrored byte-for-byte by internal/wire/source_client.go.
static_assert(sizeof(STRUCT_SELCHAR) == 904, "source STRUCT_SELCHAR ABI changed");
static_assert(sizeof(STRUCT_MOB) == 1040, "source STRUCT_MOB ABI changed");
static_assert(sizeof(STRUCT_EXT1) == 288, "source STRUCT_EXT1 ABI changed");
static_assert(sizeof(STRUCT_EXT2) == 360, "source STRUCT_EXT2 ABI changed");
static_assert(sizeof(MSG_CNFAccountLogin) == 1992, "source character-list packet ABI changed");
static_assert(offsetof(MSG_CNFAccountLogin, SecretCode) == 12, "source character-list SecretCode offset changed");
static_assert(offsetof(MSG_CNFAccountLogin, SelChar) == 32, "source character-list SelChar offset changed");
static_assert(offsetof(MSG_CNFAccountLogin, Cargo) == 936, "source character-list cargo offset changed");
static_assert(sizeof(MSG_CNFNewCharacter) == 920, "source create-character response ABI changed");
static_assert(sizeof(MSG_CNFDeleteCharacter) == 920, "source delete-character response ABI changed");
static_assert(sizeof(MSG_CNFCharacterLogin) == 1728, "source enter-world packet ABI changed");
// Character slot and runtime ClientID are adjacent but have different owners;
// lock both offsets so selection cannot silently become the scene ID again.
static_assert(offsetof(MSG_CNFCharacterLogin, Slot) == 1056, "source selected-slot offset changed");
static_assert(offsetof(MSG_CNFCharacterLogin, ClientID) == 1058, "source runtime ClientID offset changed");
// Returning from FieldScene can enter through the same 0x114 structure, so
// both scene handlers rely on these native short counters until 0x337 supplies
// their authoritative DWORD sidecars.  The offsets below are derived from the
// current source definitions (including the 8-byte STRUCT_ITEM ABI); keeping
// them explicit prevents a newer source merge from silently shifting the
// counters that the 7.48 character-list and field packets consume.
static_assert(offsetof(STRUCT_MOB, ScoreBonus) == 804, "source status-point prefix offset changed");
static_assert(offsetof(STRUCT_MOB, SpecialBonus) == 806, "source mastery-point prefix offset changed");
static_assert(offsetof(STRUCT_MOB, SkillBonus) == 808, "source skill-point prefix offset changed");
static_assert(sizeof(MSG_CreateMob) == 236, "source CreateMob packet ABI changed");
static_assert(sizeof(MSG_CreateMobTrade) == 260, "source CreateMobTrade packet ABI changed");
static_assert(sizeof(MSG_UpdateScore) == 152, "source UpdateScore packet ABI changed");
// ReqHp/ReqMp are aligned pending-cost fields, while Magician and
// LearnedSkill are distinct presentation values consumed at later stages.
static_assert(offsetof(MSG_UpdateScore, ReqHp) == 136, "source UpdateScore.ReqHp offset changed");
static_assert(offsetof(MSG_UpdateScore, ReqMp) == 140, "source UpdateScore.ReqMp offset changed");
static_assert(offsetof(MSG_UpdateScore, Magician) == 144, "source UpdateScore.Magician offset changed");
static_assert(offsetof(MSG_UpdateScore, LearnedSkill) == 148, "source UpdateScore.LearnedSkill offset changed");
static_assert(sizeof(MSG_UpdateAffect) == 268, "source UpdateAffect packet ABI changed");
static_assert(sizeof(MSG_ShopList) == 236, "source ShopList packet ABI changed");
// These field-scene packets intentionally follow the existing WYD-Go wire
// extensions rather than the imported 7.69 layouts.
static_assert(sizeof(MSG_UpdateEquip) == 60, "7.48 UpdateEquip packet ABI changed");
static_assert(sizeof(MSG_UpdateEtc) == 48, "WYD-Go UpdateEtc extension ABI changed");
// Wide progression counters occupy the tail after the native 36-byte prefix.
static_assert(offsetof(MSG_UpdateEtc, ScoreBonusWide) == 36, "UpdateEtc status sidecar offset changed");
static_assert(offsetof(MSG_UpdateEtc, SpecialBonusWide) == 40, "UpdateEtc mastery sidecar offset changed");
static_assert(offsetof(MSG_UpdateEtc, SkillBonusWide) == 44, "UpdateEtc skill sidecar offset changed");
static_assert(sizeof(MSG_SetHpMp) == 36, "WYD-Go HP/MP extension ABI changed");
// Client diagnostics are accepted by WYD-Go only in the canonical 7.48 form.
static_assert(sizeof(MSG_MessageLog) == 108, "7.48 client diagnostic packet ABI changed");
// These common world responses share the stock 7.48 layout for both clients.
static_assert(sizeof(MSG_Carry) == 528, "7.48 carry packet ABI changed");
static_assert(sizeof(MSG_SendItem) == 24, "7.48 single-item packet ABI changed");
static_assert(sizeof(MSG_CNFMobKill) == 24, "7.48 kill confirmation ABI changed");
static_assert(sizeof(MSG_RemoveMob) == 16, "7.48 remove-mob packet ABI changed");
static_assert(sizeof(MSG_CreateItem) == 32, "7.48 ground-item creation ABI changed");
static_assert(sizeof(MSG_CNFDropItem) == 28, "7.48 drop confirmation ABI changed");
static_assert(sizeof(MSG_CNFGetItem) == 28, "7.48 pickup confirmation ABI changed");
static_assert(sizeof(MSG_AddParty) == 40, "7.48 party-member packet ABI changed");
