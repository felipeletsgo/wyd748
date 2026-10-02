#pragma once
// Named colors, capacity limits, opcodes and the base parameter packets. Split from Basedef.h in original order; include "Basedef.h" for the whole set.

#include <rpc.h>
#include <iostream>
#include <io.h>
#include <fcntl.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <tchar.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <timeapi.h>
#include "UiLayout.h"
#include "../wire/CharacterLoginPacket.h"
#include "../wire/CharacterTransferPacket.h"
#include "../wire/SendItemContract.h"
#include "../wire/PickupConfirmationContract.h"
#include "../wire/DropConfirmationContract.h"
#include "../wire/GroundItemCreateContract.h"
#include "../wire/GroundItemStateContract.h"
#include "../wire/InstanceCounterContract.h"
#include "../wire/WorldStateParameterContract.h"
#include "../wire/HpMpContract.h"
#include "../wire/UpdateScoreContract.h"
#include "../wire/CarrySnapshotContract.h"
#include "../wire/UpdateEquipContract.h"
#include "../wire/UpdateAffectContract.h"
#include "../wire/CreateMobContract.h"
#include "../wire/ShopListContract.h"
#include "../wire/ItemSoldContract.h"
#include "../wire/PlayerChallengeContract.h"
#include "../wire/CombineCompleteContract.h"
#include "../wire/TradeSessionContract.h"
#include "../wire/TradeCheckConfirmationContract.h"
#include "../wire/WarInfoContract.h"
#include "../wire/ShortSkillSnapshotContract.h"
#include "../wire/ActionFrameContract.h"
#include "../wire/AttackFrameContract.h"
#include "../wire/AirMoveContract.h"
#include "../wire/InventoryTransactionContract.h"
#include "../wire/LegacySalePacket.h"
#include "../wire/CargoGoldTransferContract.h"
#include "../wire/AutoTradeContract.h"
#include "../wire/CapsuleInfoContract.h"
#include "../wire/MessagePanelPacket.h"
#include "../wire/LegacySceneMessagePacket.h"
#include "../wire/ChatMessagePacket.h"
#include "../wire/WhisperMessagePacket.h"
#include "../wire/CharacterLogoutConfirmPacket.h"
#include "../wire/CharacterLogoutRequestPacket.h"
#include "../wire/CharacterLoginConfirmContract.h"
#include "../wire/LoginPacketContract.h"
#include "../wire/ClientIntegrityArrayContract.h"
#include "../wire/TotoPurchasePacket.h"
#include "../wire/ApplyBonusPacket.h"
#include "../wire/UseItemPacket.h"
#include "../wire/PKModePacket.h"
#include "../wire/DelayStartPacket.h"
#include "../wire/BillingNoticePacket.h"
#include "../wire/PartyAddPacket.h"
#include "../wire/PartyRemovePacket.h"
#include "../wire/PartyRequestPacket.h"
#include "../wire/PartyAcceptPacket.h"
#include "../wire/MotionPacket.h"
#include "../wire/MissingEntityRequestPacket.h"
#include "../wire/RestartRecallPacket.h"
#include "../wire/KeepalivePingPacket.h"
#include "../wire/ChangeCityPacket.h"
#include "../wire/ReqTeleportPacket.h"
#include "../wire/UseNPCPacket.h"
#include "../wire/GuildDeprivatePacket.h"
#include "../wire/ChallengeConfirmPacket.h"
#include "../wire/GuildChallengePromptPacket.h"
#include "../wire/GuildRelationPacket.h"
#include "../wire/PremiumFireworkPacket.h"
#include "../wire/PremiumFireworkUsePacket.h"
#include "../wire/GamblePacket.h"
#include "../wire/MobKillConfirmPacket.h"
#include "../wire/UpdateEtcPacket.h"
#include "../wire/IndexedMessageContract.h"
#include "../wire/ServerMigrationPacket.h"
#include "../wire/ServerWarLetterContract.h"

// Basedef remains the compatibility facade. Scenes, UI, entities, and transport
// consume the types below; static assertions must accompany changes to their
// order, size, or signedness.


#define Snow                    0xFFFFFAFA
#define GhostWhite              0xFFF8F8FF
#define WhiteSmoke              0xFFF5F5F5
#define Gainsboro               0xFFDCDCDC
#define FloralWhite             0xFFFFFAF0
#define OldLace                 0xFFFDF5E6
#define Linen                   0xFFFAF0E6
#define AntiqueWhite            0xFFFAEBD7
#define PapayaWhip              0xFFFFEFD5
#define BlanchedAlmond			0xFFFFEBCD
#define Bisque					0xFFFFE4C4
#define PeachPuff				0xFFFFDAB9
#define NavajoWhite				0xFFFFDEAD
#define Moccasin				0xFFFFE4B5
#define Cornsilk				0xFFFFF8DC
#define Ivory					0xFFFFFFF0
#define LemonChiffon			0xFFFFFACD
#define Seashell				0xFFFFF5EE
#define Honeydew				0xFFF0FFF0
#define MintCream				0xFFF5FFFA
#define Azure					0xFFF0FFFF
#define AliceBlue				0xFFF0F8FF
#define lavender				0xFFE6E6FA
#define LavenderBlush			0xFFFFF0F5
#define MistyRose				0xFFFFE4E1
#define White					0xFFFFFFFF
#define Black					0xFF000000
#define DarkSlateGray			0xFF2F4F4F
#define DimGrey					0xFF696969
#define SlateGrey				0xFF708090
#define LightSlateGray			0xFF778899
#define Grey					0xFFBEBEBE
#define LightGray				0xFFD3D3D3
#define MidnightBlue			0xFF191970
#define NavyBlue				0xFF000080
#define CornflowerBlue			0xFF6495ED
#define DarkSlateBlue			0xFF483D8B
#define SlateBlue				0xFF6A5ACD
#define MediumSlateBlue			0xFF7B68EE
#define LightSlateBlue			0xFF8470FF
#define MediumBlue				0xFF0000CD
#define RoyalBlue				0xFF4169E1
#define Blue					0xFF0000FF
#define DodgerBlue				0xFF1E90FF
#define DeepSkyBlue				0xFF00BFFF
#define SkyBlue					0xFF87CEEB
#define LightSkyBlue			0xFF87CEFA
#define SeaGreen				0xFF2E8B57
#define MediumSeaGreen			0xFF3CB371
#define LightSeaGreen			0xFF20B2AA
#define PaleGreen				0xFF98FB98
#define SpringGreen				0xFF00FF7F
#define LawnGreen				0xFF7CFC00
#define Green					0xFF00FF00
#define Yellow					0xFFFFFF00

// Inventory limits and tables shared by the client and server.
#define MAX_EQUIPITEM 18
constexpr int MAX_CARGO = 128;
constexpr auto MAX_CARRY = 64;
// WYD 7.48 Ghidra FUN_0052a737 addresses Carry as a 9x7 grid and therefore
// exposes slots 0..62; only structural slot 63 stays outside the legacy UI.
constexpr auto MAX_VISIBLE_CARRY = MAX_CARRY - 1;
static_assert(MAX_VISIBLE_CARRY == 63, "WYD 7.48 inventory must expose 63 slots");
static_assert(MAX_VISIBLE_CARRY == kPickupVisibleCarrySlotCount,
	"pickup confirmation must use the visible 7.48 Carry capacity");
static_assert(MAX_VISIBLE_CARRY == kDropVisibleCarrySlotCount,
	"drop confirmation must use the visible 7.48 Carry capacity");
static_assert(kDropUsableCargoSlotCount == 120,
	"drop confirmation must preserve the usable 7.48 Cargo capacity");
constexpr int MAX_STRING = 2000;
constexpr int MAX_STRING_LENGTH = 128;

constexpr auto TM_CONNECTION_PORT = 8281;

constexpr int MAX_SERVER = 10; // Max number of game servers that can connect to DB server
constexpr int MAX_SERVERGROUP = 10;	// Max number of servers that can exist
constexpr int MAX_SERVERNUMBER = (MAX_SERVER + 1); // DB + TMSrvs + BISrv
constexpr int MAX_ITEMLIST = 6500;
static_assert(MAX_ITEMLIST == kGroundItemDefinitionCount,
	"ground-item packet must use the loaded 7.48 ItemList capacity");
using MSG_STANDARDPARM = MSG_SetPKMode;
static_assert(sizeof(MSG_STANDARDPARM) == kInstanceCounterPacketSize,
	"instance counter packets must preserve the 7.48 standard parameter ABI");
static_assert(offsetof(MSG_STANDARDPARM, Parm) == kInstanceCounterValueOffset,
	"instance counter value offset changed");
static_assert(sizeof(MSG_STANDARDPARM) == kWorldStateParameterPacketSize,
	"world state parameter packets must preserve the 7.48 ABI");
static_assert(offsetof(MSG_STANDARDPARM, Parm) == kWorldStateParameterValueOffset,
	"world state parameter value offset changed");
static_assert(sizeof(MSG_STANDARDPARM) == kCargoGoldTransferPacketSize,
	"cargo gold transfer packets must preserve the 7.48 ABI");
static_assert(offsetof(MSG_STANDARDPARM, Parm) == kCargoGoldTransferAmountOffset,
	"cargo gold transfer amount offset changed");
static_assert(sizeof(MSG_STANDARDPARM) == kServerWarLetterPacketSize,
	"server-war letter packets must preserve the 7.48 ABI");
static_assert(offsetof(MSG_STANDARDPARM, Parm) == kServerWarTargetChannelOffset,
	"server-war target channel offset changed");
constexpr int MAX_SPELL_LIST = 248;
constexpr int MAX_GUILDZONE = 5;

constexpr auto MAX_TRADE = 15;

constexpr auto MAX_EFFECT_STRING_TABLE = 50;
constexpr auto MAX_SUB_EFFECT_STRING_TABLE = 10;

constexpr auto MAX_ITEM_PRICE_REPLACE = 100;

constexpr auto MSG_RequestCapsuleInfo_Opcode = 0x2CD;
struct MSG_Exp_MsgPanel
{
	MSG_STANDARD Header;
	char Msg[128];
	int Color32;
};

struct MSG_SendInfoPlay
{
	MSG_STANDARD Header;
	int ExpBonus;
	int DropBonus;
	int AbsDamage;
	int PerfuDamage;
	int Cash;

};
constexpr auto MSG_DeleteItem_Opcode = 0x2E4;
constexpr auto MSG_SplitItem_Opcode = 0x2E5;
constexpr auto MSG_InviteGuild_Opcode = 0x3D5;
struct MSG_STANDARDPARM2
{
	MSG_STANDARD Header;
	INT32 Parm1;
	INT32 Parm2;
};
static_assert(sizeof(MSG_STANDARDPARM2) == kAirMovePacketSize, "WYD 7.48 AirMove packet must be 20 bytes");
static_assert(offsetof(MSG_STANDARDPARM2, Parm1) == kAirMoveRouteOffset, "WYD 7.48 AirMove route offset changed");
static_assert(offsetof(MSG_STANDARDPARM2, Parm2) == kAirMoveModeOffset, "WYD 7.48 AirMove mode offset changed");
static_assert(sizeof(MSG_STANDARDPARM2) == kItemSoldPacketSize,
	"ItemSold two-parameter packet size changed");
static_assert(offsetof(MSG_STANDARDPARM2, Parm1) == kItemSoldEntityOffset,
	"ItemSold entity offset changed");
static_assert(offsetof(MSG_STANDARDPARM2, Parm2) == kItemSoldPositionOffset,
	"ItemSold position offset changed");
static_assert(sizeof(MSG_STANDARDPARM2) == kPlayerChallengePacketSize,
	"PlayerChallenge two-parameter packet size changed");
static_assert(offsetof(MSG_STANDARDPARM2, Parm1) == kPlayerChallengePlayerIdOffset,
	"PlayerChallenge player ID offset changed");
static_assert(offsetof(MSG_STANDARDPARM2, Parm2) == kPlayerChallengeModeOffset,
	"PlayerChallenge mode offset changed");
static_assert(sizeof(MSG_STANDARDPARM) == kCombineCompletePacketSize,
	"CombineComplete standard parameter size changed");
static_assert(offsetof(MSG_STANDARDPARM, Parm) == kCombineCompleteResultOffset,
	"CombineComplete result offset changed");
static_assert(sizeof(MSG_STANDARDPARM2) == 20, "WYD 7.48 gamble request must be 20 bytes");

struct MSG_STANDARDPARM3
{
	MSG_STANDARD Header;
	int Parm1;
	int Parm2;
	int Parm3;
};
static_assert(sizeof(MSG_STANDARDPARM3) == kWarInfoPacketSize,
	"WarInfo full snapshot size changed");
static_assert(offsetof(MSG_STANDARDPARM3, Parm1) == kWarInfoGuildOffset,
	"WarInfo guild offset changed");
static_assert(offsetof(MSG_STANDARDPARM3, Parm2) == kWarInfoClanOffset,
	"WarInfo clan offset changed");
static_assert(offsetof(MSG_STANDARDPARM3, Parm3) == kWarInfoAllyOffset,
	"WarInfo ally offset changed");

struct MSG_TowerWar
{
	MSG_STANDARD Header;

	char Name[5][16];
	int Point[5];
};

struct		stWaterScrollMacro
{
	MSG_STANDARD Header;

	int Parm;
	int PosX;
	int PosY;
};
