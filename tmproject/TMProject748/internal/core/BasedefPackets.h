#pragma once
// Wire packets (MSG_*) and their layout checks. Split from Basedef.h in original order; include "Basedef.h" for the whole set.
#include "BasedefStructs.h"

struct MSG_Action
{
	MSG_STANDARD Header;
	short PosX;
	short PosY;
	// The 7.48 wire stores Speed before Effect and places the destination
	// before Route[24].  Keeping the newer 7.59 member order produced a valid
	// 52-byte frame with every semantic field read from the wrong offset.
	int Speed;
	int Effect;
	unsigned short TargetX;
	unsigned short TargetY;
	char Route[24];
};
// Ghidra FUN_00524bbb builds the 7.48 movement frame with these exact offsets.
// Compile-time checks prevent a future struct edit or packing change from
// silently recreating the malformed destination/route packets seen in-game.
static_assert(sizeof(MSG_Action) == kActionPacketSize,
	"WYD 7.48 MSG_Action must be 52 bytes");
static_assert(offsetof(MSG_Action, PosX) == kActionPositionOffset,
	"WYD 7.48 PosX offset mismatch");
static_assert(offsetof(MSG_Action, Speed) == kActionSpeedOffset,
	"WYD 7.48 Speed offset mismatch");
static_assert(offsetof(MSG_Action, Effect) == kActionEffectOffset,
	"WYD 7.48 Effect offset mismatch");
static_assert(offsetof(MSG_Action, TargetX) == kActionTargetOffset,
	"WYD 7.48 TargetX offset mismatch");
static_assert(offsetof(MSG_Action, Route) == kActionRouteOffset,
	"WYD 7.48 Route offset mismatch");
static_assert(sizeof(MSG_Action::Route) == kActionRouteSize,
	"WYD 7.48 Route size mismatch");

struct st_DropListMobSelected
{
	char Name[16];

	int LastSlot;

	int Slot[35];
};

struct stDropList
{
	char name[16];

	short X;

	short Y;

	long exp;

	long gold;

	STRUCT_ITEM carry[64];
};


struct MSG_DropList
{
	MSG_STANDARD Header;

	int amount;

	struct _Drop
	{
		char name[16];

		short X;

		short Y;

		int exp;

		int gold;

		STRUCT_ITEM carry[64];

	}Drop[10];

};


struct MSG_CAPSULEINFO
{
	MSG_STANDARD Header;
	unsigned int CIndex;
	short Class;
	short Level;
	short sStr;
	short sInt;
	short sDex;
	short sCon;
	short Mastery[kCapsuleInfoMasteryCount];
	short skill[9];
	short Quest;
};

struct MSG_Trade
{
	MSG_STANDARD Header;
	STRUCT_ITEM Item[15];
	char CarryPos[15];
	int TradeMoney;
	char MyCheck;
	unsigned short OpponentID;
};

constexpr auto MSG_CombineItem_Opcode = 0x3A6;
// Stock WYD 7.48 assigns one 84-byte combine request to each native artisan
// panel.  Keep these opcodes explicit instead of routing them through the
// later generic ItemMix protocol, whose UI and recipe ABI do not exist here.
constexpr auto MSG_CombineItemAylin_Opcode = 0x3B5;
constexpr auto MSG_CombineItemAgatha_Opcode = 0x3BA;
constexpr auto MSG_CombineItemTiny_Opcode = 0x3C0;
// WYD 7.48 FUN_00463aa8 sends the native seven-slot Lindy panel as the
// standard 84-byte combine packet with opcode 0x2C3.
constexpr auto MSG_CombineItemLindy_Opcode = 0x2C3;
constexpr auto MSG_CombineItemLindyAlt_Opcode = 0x2C4;
constexpr auto MSG_CombineItemOdin_Opcode = 0x2D2;
constexpr auto MSG_CombineItemEhre_Opcode = 0x2D3;
struct MSG_CombineItem
{
	MSG_STANDARD Header;
	STRUCT_ITEM Item[8];
	char CarryPos[8];
};

struct MSG_Mission
{
	MSG_STANDARD Header;
	int MissionNo;
	STRUCT_ITEM Item[8];
	char CarryPos[8];
};

struct MSG_CNFAccountLogin
{
	MSG_STANDARD Header;
	char SecretCode[16];
	STRUCT_SELCHAR SelChar;
	STRUCT_ITEM Cargo[128];
	int Coin;
	char AccountName[16];
	int SSN1;
	int SSN2;
};

struct MSG_CAPSULEUSEITEM
{
	MSG_STANDARD Header;
	int SourType;
	int SourPos;
	int DestType;
	int DestPos;
	unsigned short GridX;
	unsigned short GridY;
	unsigned short ItemID;
	char NewMobname[16];
};

struct MSG_AutoTrade
{
	MSG_STANDARD Header;
	char Desc[24];
	STRUCT_ITEM Item[12];
	BYTE CarryPos[12];
	int TradeMoney[12];
	unsigned short Tax;
	unsigned short TargetID;
};

struct MSG_MOVESTOP
{
	MSG_STANDARD Header;
	int NextX;
	int NextY;
	int CurrentX;
	int CurrentY;
	int LastX;
	int LastY;
};


struct MSG_SendItem
{
	MSG_STANDARD Header;
	short DestType;
	short DestPos;
	STRUCT_ITEM Item;
};
static_assert(sizeof(MSG_SendItem) == kSendItemPacketSize, "SendItem envelope changed");
static_assert(offsetof(MSG_SendItem, DestType) == 12, "SendItem destination type moved");
static_assert(offsetof(MSG_SendItem, DestPos) == 14, "SendItem destination slot moved");
static_assert(offsetof(MSG_SendItem, Item) == 16, "SendItem payload moved");

struct MSG_UpdateEquip
{
	MSG_STANDARD Header;
	// The 7.48 wire ABI exposes 16 equipment visuals even though this rebuilt
	// client keeps two newer local slots. Network handlers must never read them.
	unsigned short sEquip[16];
	char Equip2[16];
};
static_assert(sizeof(MSG_UpdateEquip) == kUpdateEquipPacketSize,
	"WYD 7.48 UpdateEquip packet size changed");
static_assert(offsetof(MSG_UpdateEquip, sEquip) == kUpdateEquipVisualOffset,
	"WYD 7.48 UpdateEquip visual offset changed");
static_assert(sizeof(MSG_UpdateEquip::sEquip) ==
	kUpdateEquipSlotCount * kUpdateEquipVisualSize,
	"WYD 7.48 UpdateEquip visual array changed");
static_assert(offsetof(MSG_UpdateEquip, Equip2) == kUpdateEquipAncientOffset,
	"WYD 7.48 UpdateEquip ancient offset changed");
static_assert(sizeof(MSG_UpdateEquip::Equip2) ==
	kUpdateEquipSlotCount * kUpdateEquipAncientSize,
	"WYD 7.48 UpdateEquip ancient array changed");

struct MSG_UpdateAffect
{
	MSG_STANDARD Header;
	// WYD.exe 7.48 FUN_0052b72a consumes exactly 16 eight-byte affects; keeping
	// the newer 32-slot tail changes opcode 0x3B9 from 140 to 268 bytes.
	STRUCT_AFFECT Affect[16];
};
static_assert(sizeof(MSG_UpdateAffect) == kUpdateAffectPacketSize,
	"WYD 7.48 UpdateAffect packet size changed");
static_assert(offsetof(MSG_UpdateAffect, Affect) == kUpdateAffectArrayOffset,
	"WYD 7.48 UpdateAffect array offset changed");
static_assert(sizeof(MSG_UpdateAffect::Affect) ==
	kUpdateAffectCount * kUpdateAffectEntrySize,
	"WYD 7.48 UpdateAffect array changed");

constexpr auto MSG_AccountLogin_Opcode = 0x20D;


constexpr auto MSG_MessageShout_Opcode = 0xD1D;


constexpr auto MSG_Encode_Opcode = 0xBFF;
struct MSG_Encode
{
	MSG_STANDARD Header;
	int Parm[42];
};




struct MSG_UpdateScore
{
	MSG_STANDARD Header;
	STRUCT_SCORE Score;
	unsigned short Affect[32];
	unsigned short Guild;
	unsigned short GuildLevel;
	int ReqHp;
	int ReqMp;
	char LearnedSkill;
};
static_assert(sizeof(MSG_UpdateScore) == kUpdateScorePacketSize,
	"coordinated client/server UpdateScore packet size changed");
static_assert(offsetof(MSG_UpdateScore, Score) == kUpdateScoreScoreOffset,
	"UpdateScore score offset changed");
static_assert(offsetof(MSG_UpdateScore, Affect) == kUpdateScoreAffectOffset,
	"UpdateScore affect offset changed");
static_assert(sizeof(MSG_UpdateScore::Affect) ==
	kUpdateScoreAffectCount * sizeof(unsigned short), "UpdateScore affect count changed");
static_assert(offsetof(MSG_UpdateScore, Guild) == kUpdateScoreGuildOffset,
	"UpdateScore guild offset changed");
static_assert(offsetof(MSG_UpdateScore, GuildLevel) == kUpdateScoreGuildLevelOffset,
	"UpdateScore guild level offset changed");
static_assert(offsetof(MSG_UpdateScore, ReqHp) == kUpdateScoreReqHpOffset,
	"UpdateScore pending HP cost offset changed");
static_assert(offsetof(MSG_UpdateScore, ReqMp) == kUpdateScoreReqMpOffset,
	"UpdateScore pending MP cost offset changed");
static_assert(offsetof(MSG_UpdateScore, LearnedSkill) == kUpdateScoreLearnedSkillOffset,
	"UpdateScore avatar skill selector offset changed");

// Server-to-client playtime notification. Its payload is the canonical
// MSG_STANDARDPARM::Parm value expressed in seconds.
constexpr auto MSG_ChinaPlaytime_Opcode = 0x7DB;

constexpr auto MSG_NewCharacter_Opcode = 0x20F;
struct MSG_NewCharacter
{
	MSG_STANDARD Header;
	int Slot;
	char MobName[16];
	int Class;
};

constexpr auto MSG_DeleteCharacter_Opcode = 0x211;
struct MSG_DeleteCharacter
{
	MSG_STANDARD Header;
	int Slot;
	char MobName[16];
	// Stock 7.48 sends the same 12-byte password field used by account login.
	char Password[12];
};

struct MSG_CNFNewCharacter
{
	MSG_STANDARD Header;
	STRUCT_SELCHAR SelChar;
};

struct MSG_CNFDeleteCharacter
{
	MSG_STANDARD Header;
	STRUCT_SELCHAR SelChar;
};

struct MSG_CNFCharacterLogin
{
	MSG_STANDARD Header;
	short PosX;
	short PosY;
	STRUCT_MOB MOB;
	unsigned short Slot;
	unsigned short ClientID;
	unsigned short Weather;
	char ShortSkill[16];
	STRUCT_EXT1 Ext1;
	STRUCT_EXT2 Ext2;
};

constexpr auto MSG_InitGuldName_Opcode = 0x1D6;
struct MSG_INITGULDNAME
{
	MSG_STANDARD Header;
	int Parm;
	char GuildName[12];
};

constexpr auto MSG_MessageLog_Opcode = 0x2BC;
struct MSG_MessageLog
{
	MSG_STANDARD Header;
	// The original 7.48 client sends a 108-byte diagnostic packet: the
	// 12-byte header followed by 96 text bytes. Keeping this exact boundary
	// prevents the rebuilt client from being rejected after entering the world.
	char String[96];
};

struct MSG_CreateMob
{
	MSG_STANDARD Header;
	short PosX;
	short PosY;
	unsigned short MobID;
	char MobName[16];
	unsigned short Equip[MAX_EQUIPITEM];
	unsigned short Affect[32];
	unsigned short Guild;
	char GuildLevel;
	STRUCT_SCORE Score;
	unsigned short CreateType;
	char Equip2[MAX_EQUIPITEM];
	char Nick[26];
	char Server;
};
static_assert(sizeof(MSG_CreateMob) == kCreateMobPacketSize,
	"coordinated CreateMob packet size changed");
static_assert(offsetof(MSG_CreateMob, PosX) == kCreateMobPositionOffset,
	"CreateMob position offset changed");
static_assert(offsetof(MSG_CreateMob, MobID) == kCreateMobIdOffset,
	"CreateMob entity ID offset changed");
static_assert(offsetof(MSG_CreateMob, MobName) == kCreateMobNameOffset &&
	sizeof(MSG_CreateMob::MobName) == kCreateMobNameSize,
	"CreateMob name block changed");
static_assert(offsetof(MSG_CreateMob, Equip) == kCreateMobEquipOffset &&
	sizeof(MSG_CreateMob::Equip) == kCreateMobEquipCount * sizeof(unsigned short),
	"CreateMob equip block changed");
static_assert(offsetof(MSG_CreateMob, Affect) == kCreateMobAffectOffset &&
	sizeof(MSG_CreateMob::Affect) == kCreateMobAffectCount * sizeof(unsigned short),
	"CreateMob affect block changed");
static_assert(offsetof(MSG_CreateMob, Guild) == kCreateMobGuildOffset &&
	offsetof(MSG_CreateMob, GuildLevel) == kCreateMobGuildLevelOffset,
	"CreateMob guild block changed");
static_assert(offsetof(MSG_CreateMob, Score) == kCreateMobScoreOffset &&
	sizeof(MSG_CreateMob::Score) == kCreateMobScoreSize,
	"CreateMob score block changed");
static_assert(offsetof(MSG_CreateMob, CreateType) == kCreateMobTypeOffset,
	"CreateMob type offset changed");
static_assert(offsetof(MSG_CreateMob, Equip2) == kCreateMobAncientOffset,
	"CreateMob ancient block changed");
static_assert(offsetof(MSG_CreateMob, Nick) == kCreateMobNickOffset &&
	sizeof(MSG_CreateMob::Nick) == kCreateMobNickSize,
	"CreateMob nickname block changed");
static_assert(offsetof(MSG_CreateMob, Server) == kCreateMobServerOffset,
	"CreateMob server byte offset changed");

struct MSG_CreateMobTrade
{
	MSG_STANDARD Header;
	short PosX;
	short PosY;
	unsigned short MobID;
	char MobName[16];
	unsigned short Equip[MAX_EQUIPITEM];
	unsigned short Affect[32];
	unsigned short Guild;
	char GuildLevel;
	STRUCT_SCORE Score;
	unsigned short CreateType;
	char Equip2[MAX_EQUIPITEM];
	char Nick[26];
	char Desc[24];
	char Server;
};
static_assert(sizeof(MSG_CreateMobTrade) == kCreateMobTradePacketSize,
	"coordinated CreateMobTrade packet size changed");
static_assert(offsetof(MSG_CreateMobTrade, Desc) == kCreateMobTradeDescriptionOffset &&
	sizeof(MSG_CreateMobTrade::Desc) == kCreateMobTradeDescriptionSize,
	"CreateMobTrade description block changed");
static_assert(offsetof(MSG_CreateMobTrade, Server) == kCreateMobTradeServerOffset,
	"CreateMobTrade server byte offset changed");

struct MSG_SetShortSkill
{
	MSG_STANDARD Header;
	char Skill[20];
};


struct MSG_ShopList
{
	MSG_STANDARD Header;
	int ShopType;
	STRUCT_ITEM List[27];
	int Tax;
};
static_assert(sizeof(MSG_ShopList) == kShopListPacketSize,
	"WYD 7.48 ShopList packet size changed");
static_assert(offsetof(MSG_ShopList, ShopType) == kShopListTypeOffset,
	"ShopList type offset changed");
static_assert(offsetof(MSG_ShopList, List) == kShopListItemsOffset &&
	sizeof(MSG_ShopList::List) == kShopListItemCount * kShopListItemSize,
	"ShopList item block changed");
static_assert(offsetof(MSG_ShopList, Tax) == kShopListTaxOffset,
	"ShopList tax offset changed");

constexpr auto MSG_CloseShop_Opcode = 0x196;

struct MSG_SwapItem
{
	MSG_STANDARD Header;
	char SourType;
	char SourPos;
	char DestType;
	char DestPos;
	unsigned short TargetID;
	// The native client transmits a 20-byte structure; this explicit tail makes
	// that contract independent from compiler alignment choices.
	unsigned short Reserved;
};

constexpr auto MSG_REQShopList_Opcode = 0x27B;
struct MSG_REQShopList
{
	MSG_STANDARD Header;
	unsigned short TargetID;
	// ClickOK occupies the final WORD in the observed 7.48 request. WYD-Go
	// accepts -1 and revalidates the NPC/range server-side.
	short ClickOK;
};

struct MSG_Buy
{
	MSG_STANDARD Header;
	unsigned short TargetID;
	short TargetCarryPos;
	short MyCarryPos;
	int Coin;
};


struct MSG_Attack
{
	MSG_STANDARD Header;
	// Canonical 7.48 attack prefix. The imported client had a newer 44-byte
	// prefix whose 64-bit EXP alignment moved SkillIndex and every target.
	unsigned short AttackerID;
	// The native client also reuses byte 14 as its 10-bit hit-sequence cursor.
	unsigned short Progress;
	unsigned short PosX;
	unsigned short PosY;
	unsigned short TargetX;
	unsigned short TargetY;
	short SkillIndex;
	unsigned short CurrentMp;
	char Motion;
	char SkillParm;
	char FlagLocal;
	char DoubleCritical;
	unsigned int CurrentExp;
	unsigned short ReqMp;
	unsigned short Rsv;
	int FakeExp;
	STRUCT_DAM Dam[13];
};

struct MSG_AttackTwo
{
	MSG_STANDARD Header;
	// Keep this prefix byte-identical to MSG_Attack; only target capacity differs.
	unsigned short AttackerID;
	unsigned short Progress;
	unsigned short PosX;
	unsigned short PosY;
	unsigned short TargetX;
	unsigned short TargetY;
	short SkillIndex;
	unsigned short CurrentMp;
	char Motion;
	char SkillParm;
	char FlagLocal;
	char DoubleCritical;
	unsigned int CurrentExp;
	unsigned short ReqMp;
	unsigned short Rsv;
	int FakeExp;
	STRUCT_DAM Dam[2];
};

struct MSG_AttackOne
{
	MSG_STANDARD Header;
	// Keep this prefix byte-identical to MSG_Attack; only target capacity differs.
	unsigned short AttackerID;
	unsigned short Progress;
	unsigned short PosX;
	unsigned short PosY;
	unsigned short TargetX;
	unsigned short TargetY;
	short SkillIndex;
	unsigned short CurrentMp;
	char Motion;
	char SkillParm;
	char FlagLocal;
	char DoubleCritical;
	unsigned int CurrentExp;
	unsigned short ReqMp;
	unsigned short Rsv;
	int FakeExp;
	STRUCT_DAM Dam[1];
};

// WYD 7.48 protocol ABI audit: these are the exact frame sizes consumed or
// produced by the native executable according to the Ghidra reconstruction
// and the server's canonical wire package.  Guarding the whole interaction
// family at once prevents a newer TMProject member/alignment from silently
// breaking movement, inventory, NPC, shop, stat, chat, or combat handlers.
static_assert(sizeof(MSG_Motion) == 20, "WYD 7.48 MSG_Motion must be 20 bytes");
static_assert(sizeof(MSG_Trade) == kTradePacketSize, "WYD 7.48 MSG_Trade must be 156 bytes");
static_assert(offsetof(MSG_Trade, Item) == kTradeItemsOffset, "WYD 7.48 MSG_Trade Item offset must be 12");
static_assert(offsetof(MSG_Trade, CarryPos) == kTradeCarryPositionsOffset, "WYD 7.48 MSG_Trade CarryPos offset must be 132");
static_assert(offsetof(MSG_Trade, TradeMoney) == kTradeMoneyOffset, "WYD 7.48 MSG_Trade TradeMoney offset must be 148");
static_assert(offsetof(MSG_Trade, MyCheck) == kTradeCheckOffset, "WYD 7.48 MSG_Trade MyCheck offset must be 152");
static_assert(offsetof(MSG_Trade, OpponentID) == kTradeOpponentIdOffset, "WYD 7.48 MSG_Trade OpponentID offset must be 154");
static_assert(sizeof(MSG_CombineItem) == 84, "WYD 7.48 MSG_CombineItem must be 84 bytes");
static_assert(sizeof(MSG_UseItem) == 36, "WYD 7.48 MSG_UseItem must be 36 bytes");
static_assert(sizeof(MSG_AutoTrade) == kAutoTradePacketSize, "WYD 7.48 MSG_AutoTrade must be 196 bytes");
static_assert(offsetof(MSG_AutoTrade, Desc) == kAutoTradeDescriptionOffset, "WYD 7.48 AutoTrade Desc offset must be 12");
static_assert(sizeof(((MSG_AutoTrade*)nullptr)->Desc) == kAutoTradeDescriptionSize, "WYD 7.48 AutoTrade Desc must be 24 bytes");
static_assert(offsetof(MSG_AutoTrade, Item) == kAutoTradeItemsOffset, "WYD 7.48 AutoTrade Item offset must be 36");
static_assert(sizeof(STRUCT_ITEM) == kAutoTradeItemSize, "WYD 7.48 AutoTrade item must be 8 bytes");
static_assert(sizeof(((MSG_AutoTrade*)nullptr)->Item) / sizeof(STRUCT_ITEM) == kAutoTradeItemCount, "WYD 7.48 AutoTrade must carry 12 items");
static_assert(offsetof(MSG_AutoTrade, CarryPos) == kAutoTradeCarryPositionsOffset, "WYD 7.48 AutoTrade CarryPos offset must be 132");
static_assert(offsetof(MSG_AutoTrade, TradeMoney) == kAutoTradePricesOffset, "WYD 7.48 AutoTrade prices offset must be 144");
static_assert(sizeof(((MSG_AutoTrade*)nullptr)->TradeMoney[0]) == kAutoTradePriceSize, "WYD 7.48 AutoTrade price must be 4 bytes");
static_assert(offsetof(MSG_AutoTrade, Tax) == kAutoTradeTaxOffset, "WYD 7.48 AutoTrade Tax offset must be 192");
static_assert(offsetof(MSG_AutoTrade, TargetID) == kAutoTradeTargetIdOffset, "WYD 7.48 AutoTrade TargetID offset must be 194");
static_assert(sizeof(MSG_CAPSULEINFO) == kCapsuleInfoPacketSize, "WYD 7.48 MSG_CAPSULEINFO must be 52 bytes");
static_assert(offsetof(MSG_CAPSULEINFO, CIndex) == kCapsuleInfoIndexOffset, "WYD 7.48 CapsuleInfo CIndex offset must be 12");
static_assert(offsetof(MSG_CAPSULEINFO, Class) == kCapsuleInfoClassOffset, "WYD 7.48 CapsuleInfo Class offset must be 16");
static_assert(offsetof(MSG_CAPSULEINFO, Level) == kCapsuleInfoLevelOffset, "WYD 7.48 CapsuleInfo Level offset must be 18");
static_assert(offsetof(MSG_CAPSULEINFO, sStr) == kCapsuleInfoStrengthOffset, "WYD 7.48 CapsuleInfo strength offset must be 20");
static_assert(offsetof(MSG_CAPSULEINFO, sInt) == kCapsuleInfoIntelligenceOffset, "WYD 7.48 CapsuleInfo intelligence offset must be 22");
static_assert(offsetof(MSG_CAPSULEINFO, sDex) == kCapsuleInfoDexterityOffset, "WYD 7.48 CapsuleInfo dexterity offset must be 24");
static_assert(offsetof(MSG_CAPSULEINFO, sCon) == kCapsuleInfoConstitutionOffset, "WYD 7.48 CapsuleInfo constitution offset must be 26");
static_assert(offsetof(MSG_CAPSULEINFO, Mastery) == kCapsuleInfoMasteryOffset, "WYD 7.48 CapsuleInfo mastery offset must be 28");
static_assert(sizeof(((MSG_CAPSULEINFO*)nullptr)->Mastery) / sizeof(short) == kCapsuleInfoMasteryCount, "WYD 7.48 CapsuleInfo must carry two mastery values");
static_assert(offsetof(MSG_CAPSULEINFO, skill) == kCapsuleInfoSkillOffset, "WYD 7.48 CapsuleInfo skill offset must be 32");
static_assert(sizeof(((MSG_CAPSULEINFO*)nullptr)->skill) / sizeof(short) == kCapsuleInfoSkillCount, "WYD 7.48 CapsuleInfo must carry nine skills");
static_assert(offsetof(MSG_CAPSULEINFO, Quest) == kCapsuleInfoQuestOffset, "WYD 7.48 CapsuleInfo Quest offset must be 50");
static_assert(sizeof(MSG_MOVESTOP) == 36, "WYD 7.48 MSG_MOVESTOP must be 36 bytes");
static_assert(sizeof(MSG_SendItem) == 24, "WYD 7.48 MSG_SendItem must be 24 bytes");
static_assert(sizeof(MSG_UpdateEquip) == 60, "WYD 7.48 MSG_UpdateEquip must be 60 bytes");
static_assert(sizeof(MSG_Encode) == 180, "WYD 7.48 MSG_Encode must be 180 bytes");
static_assert(offsetof(MSG_Encode, Parm) == 12, "WYD 7.48 encode parameters offset changed");
static_assert(offsetof(MSG_Encode, Parm) + 40 * sizeof(int) == 172, "WYD 7.48 encode byte 40 offset changed");
static_assert(offsetof(MSG_Encode, Parm) + 41 * sizeof(int) == 176, "WYD 7.48 encode byte 41 offset changed");
// The stock 7.48 dispatcher rejects 0x337 unless it is exactly 36 bytes.  Full
// score state belongs to 0x336; this packet only projects the incremental WORDs.
static_assert(sizeof(MSG_UpdateEtc) == 36, "WYD 7.48 MSG_UpdateEtc must be 36 bytes");
static_assert(sizeof(MSG_CharacterLogin) == 36, "WYD 7.48 MSG_CharacterLogin must be 36 bytes");
static_assert(sizeof(MSG_NewCharacter) == 36, "WYD 7.48 MSG_NewCharacter must be 36 bytes");
static_assert(sizeof(MSG_DeleteCharacter) == 44, "WYD 7.48 MSG_DeleteCharacter must be 44 bytes");
static_assert(sizeof(MSG_MessageLog) == 108, "WYD 7.48 MSG_MessageLog must be 108 bytes");
static_assert(offsetof(MSG_MessageLog, String) == 12, "WYD 7.48 diagnostic text offset changed");
static_assert(sizeof(MSG_REQMobByID) == 16, "WYD 7.48 MSG_REQMobByID must be 16 bytes");
static_assert(sizeof(MSG_SetShortSkill) == kShortSkillSnapshotPacketSize,
	"WYD 7.48 MSG_SetShortSkill size changed");
static_assert(offsetof(MSG_SetShortSkill, Skill) == kShortSkillSnapshotSkillsOffset,
	"WYD 7.48 MSG_SetShortSkill payload offset changed");
static_assert(sizeof(MSG_SetShortSkill::Skill) == kShortSkillSnapshotSkillCount,
	"WYD 7.48 MSG_SetShortSkill payload length changed");
static_assert(sizeof(MSG_ShopList) == 236, "WYD 7.48 MSG_ShopList must be 236 bytes");
static_assert(sizeof(MSG_SwapItem) == kSwapItemPacketSize,
	"WYD 7.48 MSG_SwapItem size changed");
static_assert(offsetof(MSG_SwapItem, SourType) == kSwapItemSourceTypeOffset,
	"WYD 7.48 MSG_SwapItem SourType offset changed");
static_assert(offsetof(MSG_SwapItem, SourPos) == kSwapItemSourcePositionOffset,
	"WYD 7.48 MSG_SwapItem SourPos offset changed");
static_assert(offsetof(MSG_SwapItem, DestType) == kSwapItemDestinationTypeOffset,
	"WYD 7.48 MSG_SwapItem DestType offset changed");
static_assert(offsetof(MSG_SwapItem, DestPos) == kSwapItemDestinationPositionOffset,
	"WYD 7.48 MSG_SwapItem DestPos offset changed");
static_assert(offsetof(MSG_SwapItem, TargetID) == kSwapItemTargetIdOffset,
	"WYD 7.48 MSG_SwapItem TargetID offset changed");
static_assert(offsetof(MSG_SwapItem, Reserved) == kSwapItemReservedOffset,
	"WYD 7.48 MSG_SwapItem Reserved offset changed");
static_assert(sizeof(MSG_REQShopList) == 16, "WYD 7.48 MSG_REQShopList must be 16 bytes");
static_assert(sizeof(MSG_Buy) == kBuyPacketSize,
	"WYD 7.48 MSG_Buy size changed");
static_assert(offsetof(MSG_Buy, TargetID) == kBuyTargetIdOffset,
	"WYD 7.48 MSG_Buy TargetID offset changed");
static_assert(offsetof(MSG_Buy, TargetCarryPos) == kBuyShopPositionOffset,
	"WYD 7.48 MSG_Buy TargetCarryPos offset changed");
static_assert(offsetof(MSG_Buy, MyCarryPos) == kBuyCarryPositionOffset,
	"WYD 7.48 MSG_Buy MyCarryPos offset changed");
static_assert(offsetof(MSG_Buy, Coin) == kBuyCoinOffset,
	"WYD 7.48 MSG_Buy Coin offset changed");
static_assert(sizeof(MSG_Attack) == kAttackMultiBasePacketSize,
	"WYD 7.48 MSG_Attack size changed");
static_assert(sizeof(MSG_AttackTwo) == kAttackTwoBasePacketSize,
	"WYD 7.48 MSG_AttackTwo size changed");
static_assert(sizeof(MSG_AttackOne) == kAttackOneBasePacketSize,
	"WYD 7.48 MSG_AttackOne size changed");
static_assert(offsetof(MSG_Attack, AttackerID) == kAttackAttackerIdOffset,
	"WYD 7.48 attack AttackerID offset changed");
static_assert(offsetof(MSG_Attack, Progress) == kAttackProgressOffset,
	"WYD 7.48 attack Progress offset changed");
static_assert(offsetof(MSG_Attack, PosX) == kAttackPositionOffset,
	"WYD 7.48 attack position offset changed");
static_assert(offsetof(MSG_Attack, TargetX) == kAttackTargetPositionOffset,
	"WYD 7.48 attack target position offset changed");
static_assert(offsetof(MSG_Attack, SkillIndex) == kAttackSkillIndexOffset,
	"WYD 7.48 attack SkillIndex offset changed");
static_assert(offsetof(MSG_Attack, CurrentMp) == kAttackCurrentMpOffset,
	"WYD 7.48 attack CurrentMp offset changed");
static_assert(offsetof(MSG_Attack, Motion) == kAttackMotionOffset,
	"WYD 7.48 attack Motion offset changed");
static_assert(offsetof(MSG_Attack, SkillParm) == kAttackSkillParameterOffset,
	"WYD 7.48 attack SkillParm offset changed");
static_assert(offsetof(MSG_Attack, FlagLocal) == kAttackLocalFlagOffset,
	"WYD 7.48 attack FlagLocal offset changed");
static_assert(offsetof(MSG_Attack, DoubleCritical) == kAttackDoubleCriticalOffset,
	"WYD 7.48 attack DoubleCritical offset changed");
static_assert(offsetof(MSG_Attack, CurrentExp) == kAttackCurrentExpOffset,
	"WYD 7.48 attack CurrentExp offset changed");
static_assert(offsetof(MSG_Attack, ReqMp) == kAttackRequiredMpOffset,
	"WYD 7.48 attack ReqMp offset changed");
static_assert(offsetof(MSG_Attack, Rsv) == kAttackReservedOffset,
	"WYD 7.48 attack Rsv offset changed");
static_assert(offsetof(MSG_Attack, FakeExp) == kAttackFakeExpOffset,
	"WYD 7.48 attack FakeExp offset changed");
static_assert(offsetof(MSG_Attack, Dam) == kAttackDamagesOffset,
	"WYD 7.48 attack target list offset changed");
static_assert(sizeof(MSG_AttackOne::Dam) /
	sizeof(MSG_AttackOne::Dam[0]) == kAttackOneTargetCapacity,
	"WYD 7.48 single-target attack capacity changed");
static_assert(sizeof(MSG_AttackTwo::Dam) /
	sizeof(MSG_AttackTwo::Dam[0]) == kAttackTwoTargetCapacity,
	"WYD 7.48 two-target attack capacity changed");
static_assert(sizeof(MSG_Attack::Dam) /
	sizeof(MSG_Attack::Dam[0]) == kAttackMultiTargetCapacity,
	"WYD 7.48 multi-target attack capacity changed");
static_assert(sizeof(MSG_STANDARD) == 12, "WYD 7.48 standard header must be 12 bytes");
static_assert(offsetof(MSG_STANDARD, Size) == 0, "WYD 7.48 header Size offset changed");
static_assert(offsetof(MSG_STANDARD, KeyWord) == 2, "WYD 7.48 header KeyWord offset changed");
static_assert(offsetof(MSG_STANDARD, CheckSum) == 3, "WYD 7.48 header CheckSum offset changed");
static_assert(offsetof(MSG_STANDARD, Type) == 4, "WYD 7.48 header Type offset changed");
static_assert(offsetof(MSG_STANDARD, ID) == 6, "WYD 7.48 header ID offset changed");
static_assert(offsetof(MSG_STANDARD, Tick) == 8, "WYD 7.48 header Tick offset changed");
static_assert(sizeof(MSG_STANDARDPARM) == 16, "WYD 7.48 one-parameter packet must be 16 bytes");
static_assert(offsetof(MSG_STANDARDPARM, Parm) == 12, "WYD 7.48 Parm offset changed");
static_assert(sizeof(MSG_STANDARDPARM2) == 20, "WYD 7.48 two-parameter packet must be 20 bytes");
static_assert(offsetof(MSG_STANDARDPARM2, Parm1) == 12, "WYD 7.48 Parm1 offset changed");
static_assert(offsetof(MSG_STANDARDPARM2, Parm2) == 16, "WYD 7.48 Parm2 offset changed");
static_assert(sizeof(MSG_STANDARDPARM3) == 24, "WYD 7.48 three-parameter packet must be 24 bytes");
static_assert(offsetof(MSG_STANDARDPARM3, Parm1) == 12, "WYD 7.48 Parm1 offset changed");
static_assert(offsetof(MSG_STANDARDPARM3, Parm2) == 16, "WYD 7.48 Parm2 offset changed");
static_assert(offsetof(MSG_STANDARDPARM3, Parm3) == 20, "WYD 7.48 Parm3 offset changed");

static_assert(sizeof(MSG_CNFCharacterLogin) == kCharacterLoginConfirmPacketSize,
	"WYD 7.48 character login confirmation must be 2104 bytes");
static_assert(sizeof(MSG_CNFAccountLogin) == kAccountLoginConfirmPacketSize,
	"WYD 7.48 account login confirmation must be 2360 bytes");
static_assert(sizeof(MSG_CNFNewCharacter) == kCharacterSelectionUpdatePacketSize,
	"WYD 7.48 create-character confirmation must be 1288 bytes");
static_assert(sizeof(MSG_CNFDeleteCharacter) == kCharacterSelectionUpdatePacketSize,
	"WYD 7.48 delete-character confirmation must be 1288 bytes");

constexpr auto MSG_ReqBuy_Opcode = 0x398;
struct MSG_ReqBuy
{
	MSG_STANDARD Header;
	int Pos;
	unsigned short TargetID;
	int Price;
	int Tax;
	STRUCT_ITEM item;
};

struct MSG_RemoveMob
{
	MSG_STANDARD Header;
	int RemoveType;
};
static_assert(sizeof(MSG_RemoveMob) == kWorldStateParameterPacketSize,
	"WYD 7.48 RemoveMob packet size changed");
static_assert(offsetof(MSG_RemoveMob, RemoveType) == kWorldStateParameterValueOffset,
	"WYD 7.48 RemoveMob value offset changed");

struct MSG_SetHpMode
{
	MSG_STANDARD Header;
	int Hp;
	short Mode;
};
struct PacketRevDonate2 {
	MSG_STANDARD Header;

	int quantidade;

	struct
	{
		int type;

		int page;

		STRUCT_ITEM item;

		int price;

		int stuck;

		int slot;
	}Produts[231];

};

struct PacketRevDonate {
	MSG_STANDARD Header;

	int type;

	int page;

	STRUCT_ITEM item;

	int price;

	int stuck;

	int slot;
};

struct MSG_DAILYREWARDINFO
{
	MSG_STANDARD Header;

	// All rewards.
	STRUCT_ITEM Item[7][4];

	// Days already claimed.
	bool Received[7];

	// Current day.
	int Day;
};

struct MSG_SetHpMp
{
	MSG_STANDARD Header;
	unsigned int Hp;
	unsigned int Mp;
	unsigned int MaxHp;
	unsigned int MaxMp;
};
static_assert(sizeof(MSG_SetHpMp) == kHpMpPacketSize,
	"coordinated client/server HP/MP packet size changed");
static_assert(offsetof(MSG_SetHpMp, Hp) == kHpMpCurrentHpOffset,
	"HP/MP current HP offset changed");
static_assert(offsetof(MSG_SetHpMp, Mp) == kHpMpCurrentMpOffset,
	"HP/MP current MP offset changed");
static_assert(offsetof(MSG_SetHpMp, MaxHp) == kHpMpMaximumHpOffset,
	"HP/MP maximum HP offset changed");
static_assert(offsetof(MSG_SetHpMp, MaxMp) == kHpMpMaximumMpOffset,
	"HP/MP maximum MP offset changed");

struct MSG_SetHpDam
{
	MSG_STANDARD Header;
	int Hp;
	short Dam;
};

struct MSG_Carry
{
	MSG_STANDARD Header;
	STRUCT_ITEM Carry[64];
	int Coin;
};

static_assert(sizeof(MSG_Carry) == kCarrySnapshotPacketSize,
	"WYD 7.48 carry packet size changed");
static_assert(offsetof(MSG_Carry, Carry) == kCarrySnapshotItemsOffset,
	"WYD 7.48 carry payload offset changed");
static_assert(sizeof(MSG_Carry::Carry) ==
	kCarrySnapshotItemCount * kCarrySnapshotItemSize,
	"WYD 7.48 carry item array changed");
static_assert(offsetof(MSG_Carry, Coin) == kCarrySnapshotCoinOffset,
	"WYD 7.48 carry coin offset changed");
static_assert(MAX_VISIBLE_CARRY == kCarrySnapshotVisibleItemCount,
	"WYD 7.48 carry visual capacity changed");

struct MSG_HellBuy
{
	MSG_STANDARD Header;
	unsigned short TargetID;
	short TargetCarryPos;
	short MyCarryPos;
	int Coin;
};

struct MSG_UpdateItem
{
	MSG_STANDARD Header;
	int ItemID;
	short State;
	char Height;
	char dummy;
};

static_assert(sizeof(MSG_UpdateItem) == kGroundItemUpdatePacketSize,
	"WYD 7.48 ground-item update must be 20 bytes");
static_assert(offsetof(MSG_UpdateItem, ItemID) == kGroundItemUpdateItemIDOffset,
	"WYD 7.48 ground-item update ID offset changed");
static_assert(offsetof(MSG_UpdateItem, State) == kGroundItemUpdateStateOffset,
	"WYD 7.48 ground-item update state offset changed");
static_assert(offsetof(MSG_UpdateItem, Height) == kGroundItemUpdateHeightOffset,
	"WYD 7.48 ground-item update height offset changed");

struct MSG_ReqSummon
{
	MSG_STANDARD Header;
	int Result;
	char Name[16];
};

struct MSG_CNFRemoveServerLogin
{
	MSG_STANDARD Header;
	STRUCT_SELCHAR SelChar;
	char AccountName[16];
	STRUCT_ITEM Cargo[128];
	int Coin;
	char SecretCode[16];
	int SSN1;
	int SSN2;
	short PosX;
	short PosY;
	STRUCT_MOB MOB;
	unsigned short Slot;
	unsigned short ClientID;
	unsigned short Weather;
	char ShortSkill[16];
	STRUCT_EXT1 Ext1;
	STRUCT_EXT2 Ext2;
};

struct MSG_EnvEffect
{
	MSG_STANDARD Header;
	short x1;
	short y1;
	short x2;
	short y2;
	short Effect;
	short EffectParm;
};

struct MSG_RandomQuiz
{
	MSG_STANDARD Header;
	char Question[128];
	char Answer[4][32];
};

struct MSG_LongMessagePanel
{
	MSG_STANDARD Header;
	short Parm1;
	short Parm2;
	char Line[4][128];
};

constexpr auto MSG_DropItem_Opcode = 0x272;
struct MSG_DropItem
{
	MSG_STANDARD Header;
	int SourType;
	int SourPos;
	int Rotate;
	unsigned short GridX;
	unsigned short GridY;
	unsigned short ItemID;
};

struct MSG_CreateItem
{
	MSG_STANDARD Header;
	unsigned short GridX;
	unsigned short GridY;
	unsigned short ItemID;
	STRUCT_ITEM Item;
	char Rotate;
	char State;
	char Height;
	char Create;
	unsigned short Owner;
};

static_assert(sizeof(MSG_CreateItem) == kGroundItemCreatePacketSize,
	"WYD 7.48 ground-item creation must be 32 bytes");
static_assert(offsetof(MSG_CreateItem, GridX) == kGroundItemCreateGridXOffset,
	"WYD 7.48 ground-item X offset changed");
static_assert(offsetof(MSG_CreateItem, GridY) == kGroundItemCreateGridYOffset,
	"WYD 7.48 ground-item Y offset changed");
static_assert(offsetof(MSG_CreateItem, ItemID) == kGroundItemCreateItemIDOffset,
	"WYD 7.48 ground-item ID offset changed");
static_assert(offsetof(MSG_CreateItem, Item) == kGroundItemCreateItemOffset,
	"WYD 7.48 ground-item payload offset changed");
static_assert(offsetof(MSG_CreateItem, Rotate) == kGroundItemCreateRotateOffset,
	"WYD 7.48 ground-item rotation offset changed");
static_assert(offsetof(MSG_CreateItem, State) == kGroundItemCreateStateOffset,
	"WYD 7.48 ground-item state offset changed");
static_assert(offsetof(MSG_CreateItem, Height) == kGroundItemCreateHeightOffset,
	"WYD 7.48 ground-item height offset changed");
static_assert(offsetof(MSG_CreateItem, Create) == kGroundItemCreateFlagOffset,
	"WYD 7.48 ground-item create flag offset changed");
static_assert(offsetof(MSG_CreateItem, Owner) == kGroundItemCreateOwnerOffset,
	"WYD 7.48 ground-item owner offset changed");

struct MSG_CNFDropItem
{
	MSG_STANDARD Header;
	int SourType;
	int SourPos;
	int Rotate;
	unsigned short GridX;
	unsigned short GridY;
};

static_assert(sizeof(MSG_CNFDropItem) == kDropConfirmationPacketSize,
	"WYD 7.48 drop confirmation must be 28 bytes");
static_assert(offsetof(MSG_CNFDropItem, SourType) == kDropConfirmationSourceTypeOffset,
	"WYD 7.48 drop source type offset changed");
static_assert(offsetof(MSG_CNFDropItem, SourPos) == kDropConfirmationSourcePosOffset,
	"WYD 7.48 drop source slot offset changed");
static_assert(offsetof(MSG_CNFDropItem, Rotate) == kDropConfirmationRotateOffset,
	"WYD 7.48 drop rotation offset changed");
static_assert(offsetof(MSG_CNFDropItem, GridX) == kDropConfirmationGridXOffset,
	"WYD 7.48 drop X offset changed");
static_assert(offsetof(MSG_CNFDropItem, GridY) == kDropConfirmationGridYOffset,
	"WYD 7.48 drop Y offset changed");

struct MSG_CNFGetItem
{
	MSG_STANDARD Header;
	int DestType;
	int DestPos;
	STRUCT_ITEM Item;
};

static_assert(sizeof(MSG_CNFGetItem) == kPickupConfirmationPacketSize,
	"WYD 7.48 pickup confirmation must be 28 bytes");
static_assert(offsetof(MSG_CNFGetItem, DestType) == kPickupConfirmationDestTypeOffset,
	"WYD 7.48 pickup destination type offset changed");
static_assert(offsetof(MSG_CNFGetItem, DestPos) == kPickupConfirmationDestPosOffset,
	"WYD 7.48 pickup destination slot offset changed");
static_assert(offsetof(MSG_CNFGetItem, Item) == kPickupConfirmationItemOffset,
	"WYD 7.48 pickup item offset changed");

struct MSG_RMBShopList
{
	MSG_STANDARD Header;
	int ShopType;
	STRUCT_ITEM List[39];
	int Tax;
};

constexpr auto MSG_GetItem_Opcode = 0x270;
struct MSG_GetItem
{
	MSG_STANDARD Header;
	int DestType;
	int DestPos;
	unsigned short ItemID;
	unsigned short GridX;
	unsigned short GridY;
};
