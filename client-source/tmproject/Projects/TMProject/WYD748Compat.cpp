#include "pch.h"

#include <cstddef>

#include "Basedef.h"

// These assertions lock the canonical 7.48+ Score shared with Go. Any field
// insertion, narrowing or reordering must fail the Win32 build immediately.
static_assert(sizeof(STRUCT_SCORE) == 140, "canonical STRUCT_SCORE must remain 140 bytes");
static_assert(offsetof(STRUCT_SCORE, Version) == 0, "STRUCT_SCORE.Version offset changed");
static_assert(offsetof(STRUCT_SCORE, Level) == 4, "STRUCT_SCORE.Level offset changed");
static_assert(offsetof(STRUCT_SCORE, Attack) == 8, "STRUCT_SCORE.Attack offset changed");
static_assert(offsetof(STRUCT_SCORE, MagicAttack) == 12, "STRUCT_SCORE.MagicAttack offset changed");
static_assert(offsetof(STRUCT_SCORE, Defense) == 16, "STRUCT_SCORE.Defense offset changed");
static_assert(offsetof(STRUCT_SCORE, MaxHP) == 20, "STRUCT_SCORE.MaxHP offset changed");
static_assert(offsetof(STRUCT_SCORE, MaxMP) == 24, "STRUCT_SCORE.MaxMP offset changed");
static_assert(offsetof(STRUCT_SCORE, CurHP) == 28, "STRUCT_SCORE.CurHP offset changed");
static_assert(offsetof(STRUCT_SCORE, CurMP) == 32, "STRUCT_SCORE.CurMP offset changed");
static_assert(offsetof(STRUCT_SCORE, Str) == 36, "STRUCT_SCORE.Str offset changed");
static_assert(offsetof(STRUCT_SCORE, Int) == 40, "STRUCT_SCORE.Int offset changed");
static_assert(offsetof(STRUCT_SCORE, Dex) == 44, "STRUCT_SCORE.Dex offset changed");
static_assert(offsetof(STRUCT_SCORE, Con) == 48, "STRUCT_SCORE.Con offset changed");
static_assert(offsetof(STRUCT_SCORE, Accuracy) == 52, "STRUCT_SCORE.Accuracy offset changed");
static_assert(offsetof(STRUCT_SCORE, Evasion) == 56, "STRUCT_SCORE.Evasion offset changed");
static_assert(offsetof(STRUCT_SCORE, Parry) == 60, "STRUCT_SCORE.Parry offset changed");
static_assert(offsetof(STRUCT_SCORE, Critical) == 64, "STRUCT_SCORE.Critical offset changed");
static_assert(offsetof(STRUCT_SCORE, Range) == 68, "STRUCT_SCORE.Range offset changed");
static_assert(offsetof(STRUCT_SCORE, ResistFire) == 72, "STRUCT_SCORE.ResistFire offset changed");
static_assert(offsetof(STRUCT_SCORE, ResistIce) == 76, "STRUCT_SCORE.ResistIce offset changed");
static_assert(offsetof(STRUCT_SCORE, ResistHoly) == 80, "STRUCT_SCORE.ResistHoly offset changed");
static_assert(offsetof(STRUCT_SCORE, ResistThunder) == 84, "STRUCT_SCORE.ResistThunder offset changed");
static_assert(offsetof(STRUCT_SCORE, SaveMana) == 88, "STRUCT_SCORE.SaveMana offset changed");
static_assert(offsetof(STRUCT_SCORE, MagicAmp) == 92, "STRUCT_SCORE.MagicAmp offset changed");
static_assert(offsetof(STRUCT_SCORE, RegenHP) == 96, "STRUCT_SCORE.RegenHP offset changed");
static_assert(offsetof(STRUCT_SCORE, RegenMP) == 100, "STRUCT_SCORE.RegenMP offset changed");
static_assert(offsetof(STRUCT_SCORE, StatusPts) == 104, "STRUCT_SCORE.StatusPts offset changed");
static_assert(offsetof(STRUCT_SCORE, MasterPts) == 108, "STRUCT_SCORE.MasterPts offset changed");
static_assert(offsetof(STRUCT_SCORE, SkillPts) == 112, "STRUCT_SCORE.SkillPts offset changed");
static_assert(offsetof(STRUCT_SCORE, Mastery) == 116, "STRUCT_SCORE.Mastery offset changed");
static_assert(offsetof(STRUCT_SCORE, AttackRun) == 132, "STRUCT_SCORE.AttackRun offset changed");
static_assert(offsetof(STRUCT_SCORE, Merchant) == 136, "STRUCT_SCORE.Merchant offset changed");

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
// FUN_0055890a reads opcode 0x373 at these exact offsets. Locking every field
// prevents a newer TMProject layout from silently changing the server wire ABI.
static_assert(offsetof(MSG_UseItem, SourType) == 12, "7.48 UseItem.SourType offset changed");
static_assert(offsetof(MSG_UseItem, SourPos) == 16, "7.48 UseItem.SourPos offset changed");
static_assert(offsetof(MSG_UseItem, DestType) == 20, "7.48 UseItem.DestType offset changed");
static_assert(offsetof(MSG_UseItem, DestPos) == 24, "7.48 UseItem.DestPos offset changed");
static_assert(offsetof(MSG_UseItem, GridX) == 28, "7.48 UseItem.GridX offset changed");
static_assert(offsetof(MSG_UseItem, GridY) == 30, "7.48 UseItem.GridY offset changed");
static_assert(offsetof(MSG_UseItem, ItemID) == 32, "7.48 UseItem.ItemID offset changed");
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

// The source-client response family embeds the canonical 140-byte Score.
// These values are mirrored byte-for-byte by internal/wire/source_client.go.
static_assert(sizeof(STRUCT_SELCHAR) == 1272, "source STRUCT_SELCHAR ABI changed");
static_assert(sizeof(STRUCT_MOB) == 1224, "source STRUCT_MOB ABI changed");
static_assert(offsetof(STRUCT_MOB, ShortSkill) == 988, "source ShortSkill offset changed");
static_assert(offsetof(STRUCT_MOB, GuildLevel) == 992, "source GuildLevel offset changed");
static_assert(offsetof(STRUCT_MOB, CurrentKill) == 1220, "source CurrentKill offset changed");
static_assert(sizeof(STRUCT_EXT1) == 288, "source STRUCT_EXT1 ABI changed");
static_assert(sizeof(STRUCT_EXT2) == 552, "source STRUCT_EXT2 ABI changed");
static_assert(sizeof(MSG_CNFAccountLogin) == 2360, "source character-list packet ABI changed");
static_assert(offsetof(MSG_CNFAccountLogin, SecretCode) == 12, "source character-list SecretCode offset changed");
static_assert(offsetof(MSG_CNFAccountLogin, SelChar) == 32, "source character-list SelChar offset changed");
static_assert(offsetof(MSG_CNFAccountLogin, Cargo) == 1304, "source character-list cargo offset changed");
static_assert(sizeof(MSG_CNFNewCharacter) == 1288, "source create-character response ABI changed");
static_assert(sizeof(MSG_CNFDeleteCharacter) == 1288, "source delete-character response ABI changed");
static_assert(sizeof(MSG_CNFCharacterLogin) == 2104, "source enter-world packet ABI changed");
static_assert(offsetof(MSG_CNFCharacterLogin, Slot) == 1240, "source selected-slot offset changed");
static_assert(offsetof(MSG_CNFCharacterLogin, ClientID) == 1242, "source runtime ClientID offset changed");
static_assert(offsetof(MSG_CNFCharacterLogin, Ext1) == 1264, "source Ext1 alignment changed");
static_assert(sizeof(MSG_CreateMob) == 328, "source CreateMob packet ABI changed");
// FUN_004829f2 reads the 24-byte shop title at wire offset 326 before updating TMHuman::m_TradeDesc.
static_assert(offsetof(MSG_CreateMobTrade, Desc) == 326, "7.48 CreateMobTrade.Desc offset changed");
static_assert(sizeof(MSG_CreateMobTrade) == 352, "source CreateMobTrade packet ABI changed");
static_assert(sizeof(MSG_UpdateScore) == 232, "canonical UpdateScore packet ABI changed");
static_assert(offsetof(MSG_UpdateScore, Affect) == 152, "canonical UpdateScore.Affect offset changed");
static_assert(offsetof(MSG_UpdateScore, Guild) == 216, "canonical UpdateScore.Guild offset changed");
static_assert(offsetof(MSG_UpdateScore, ReqHp) == 220, "canonical UpdateScore.ReqHp offset changed");
static_assert(offsetof(MSG_UpdateScore, ReqMp) == 224, "canonical UpdateScore.ReqMp offset changed");
static_assert(offsetof(MSG_UpdateScore, LearnedSkill) == 228, "canonical UpdateScore.LearnedSkill offset changed");
// FUN_0052b72a proves Header(12) + 16 * STRUCT_AFFECT(8) for opcode 0x3B9.
static_assert(sizeof(MSG_UpdateAffect) == 140, "7.48 UpdateAffect packet ABI changed");
static_assert(sizeof(MSG_ShopList) == 236, "source ShopList packet ABI changed");
// Incremental notifications never carry an alternate score representation.
static_assert(sizeof(MSG_UpdateEquip) == 60, "7.48 UpdateEquip packet ABI changed");
// Ghidra FUN_0052d93d and the dispatch gate at 0x0055890a prove that 0x337 is
// exactly 36 bytes: three DWORDs, four WORDs and Coin after the standard header.
static_assert(sizeof(MSG_UpdateEtc) == 36, "7.48 UpdateEtc packet ABI changed");
static_assert(offsetof(MSG_UpdateEtc, StatusPoint) == 24, "7.48 UpdateEtc.StatusPoint offset changed");
static_assert(offsetof(MSG_UpdateEtc, MasterPoint) == 26, "7.48 UpdateEtc.MasterPoint offset changed");
static_assert(offsetof(MSG_UpdateEtc, SkillPoint) == 28, "7.48 UpdateEtc.SkillPoint offset changed");
static_assert(offsetof(MSG_UpdateEtc, Magic) == 30, "7.48 UpdateEtc.Magic offset changed");
static_assert(offsetof(MSG_UpdateEtc, Coin) == 32, "7.48 UpdateEtc.Coin offset changed");
static_assert(sizeof(MSG_SetHpMp) == 28, "canonical HP/MP incremental ABI changed");
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
