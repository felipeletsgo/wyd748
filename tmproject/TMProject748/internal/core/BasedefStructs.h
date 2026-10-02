#pragma once
// Game data structures (STRUCT_*) and their layout checks. Split from Basedef.h in original order; include "Basedef.h" for the whole set.
#include "BasedefLimits.h"

// Canonical WYD 7.48+ score shared byte-for-byte with model.Score.
// Every field is one unsigned 32-bit word; no legacy aliases are retained.
// Game state structures materialized from 7.48 data.
struct STRUCT_SCORE
{
	unsigned int Version;
	unsigned int Level;
	unsigned int Attack;
	unsigned int MagicAttack;
	unsigned int Defense;
	unsigned int MaxHP;
	unsigned int MaxMP;
	unsigned int CurHP;
	unsigned int CurMP;
	unsigned int Str;
	unsigned int Int;
	unsigned int Dex;
	unsigned int Con;
	unsigned int Accuracy;
	unsigned int Evasion;
	unsigned int Parry;
	unsigned int Critical;
	unsigned int Range;
	unsigned int ResistFire;
	unsigned int ResistIce;
	unsigned int ResistHoly;
	unsigned int ResistThunder;
	unsigned int SaveMana;
	unsigned int MagicAmp;
	unsigned int RegenHP;
	unsigned int RegenMP;
	unsigned int StatusPts;
	unsigned int MasterPts;
	unsigned int SkillPts;
	unsigned int Mastery[4];
	unsigned int AttackRun;
	unsigned int Merchant;
};

union STRUCT_BONUSEFFECT
{
	struct
	{
		unsigned char cEffect;
		unsigned char cValue;
	};
	short sValue;
};

struct STRUCT_ITEM
{
	short sIndex;
	STRUCT_BONUSEFFECT stEffect[3];
};

static_assert(sizeof(STRUCT_BONUSEFFECT) == 2, "WYD item effect must be 2 bytes");
static_assert(sizeof(STRUCT_ITEM) == 8, "WYD item must be 8 bytes");
static_assert(offsetof(STRUCT_ITEM, sIndex) == 0, "WYD item index offset changed");
static_assert(offsetof(STRUCT_ITEM, stEffect) == 2, "WYD item effects offset changed");

struct STRUCT_SELCHAR
{
	unsigned short HomeTownX[4];
	unsigned short HomeTownY[4];
	char MobName[4][16];
	STRUCT_SCORE Score[4];
	STRUCT_ITEM Equip[4][MAX_EQUIPITEM];
	unsigned short Guild[4];
	int Coin[4];
	long long Exp[4];
};

struct STRUCT_MOB
{
	char MobName[16];
	char Clan;
	unsigned short Guild;
	char Class;
	char Rsv;
	unsigned short Quest;
	int Coin;
	long long Exp;
	unsigned short HomeTownX;
	unsigned short HomeTownY;
	STRUCT_SCORE BaseScore;
	STRUCT_SCORE CurrentScore;
	STRUCT_ITEM Equip[MAX_EQUIPITEM];
	STRUCT_ITEM Carry[64];
	unsigned int LearnedSkill[2];
	char ShortSkill[4];
	char GuildLevel;
	char dummy[227];
	unsigned short CurrentKill;
	unsigned short TotalKill;
bool HasSoulSkill() const
	{
		return this->LearnedSkill[0] & 0x40000000;
	}
};

struct STRUCT_AFFECT
{
	char Type;
	char Level;
	short Value;
	int Time;
};
static_assert(sizeof(STRUCT_AFFECT) == kUpdateAffectEntrySize,
	"TMProject748 affect entry size changed");
static_assert(offsetof(STRUCT_AFFECT, Type) == kUpdateAffectTypeOffset,
	"TMProject748 affect Type offset changed");
static_assert(offsetof(STRUCT_AFFECT, Level) == kUpdateAffectLevelOffset,
	"TMProject748 affect Level offset changed");
static_assert(offsetof(STRUCT_AFFECT, Value) == kUpdateAffectValueOffset,
	"TMProject748 affect Value offset changed");
static_assert(offsetof(STRUCT_AFFECT, Time) == kUpdateAffectTimeOffset,
	"TMProject748 affect Time offset changed");

struct STRUCT_MYBONUSEFFECT
{
	char cEffect;
	char cValue;
	short sValue;
};

struct STRUCT_MYITEM
{
	short sIndex;
	STRUCT_MYBONUSEFFECT stEffect[3];
};

struct STRUCT_NEEDITEM
{
	int Access;
	int dINDEX;
};

struct STRUCT_POSITION
{
	int X;
	int Y;
};

struct STRUCT_RESULT_ITEMLIST
{
	unsigned int dNPCHead;
	STRUCT_POSITION pxy;
	STRUCT_MYITEM stItemInfor;
	unsigned short sMSG;
	STRUCT_NEEDITEM dNeedItemList[8];
	unsigned int dCost;
	char stSameList[20][128];
};

struct STRUCT_OPTION
{
	unsigned int dClass[2];
	unsigned int dPOS[2];
};

struct STRUCT_NEED_ITEMLIST
{
	int Textnum;
	unsigned int dVolume;
	STRUCT_MYITEM stGridItem;
	unsigned int dListIndexArry[6];
	STRUCT_OPTION stOption;
};

struct STRUCT_REFER
{
	unsigned int dindex;
	unsigned int dHavevolume;
	int bItemListrefer;
};

struct STRUCT_MIXHELP
{
	short Color[9];
	short Icon;
	char Help[9][128];
	char Name[128];
};

struct STRUCT_MISSIONITEM
{
	unsigned int dHavevolume;
	unsigned int dNeedvolume[2];
};

struct STRUCT_STATICEFFECT
{
	short sEffect;
	short sValue;
};

struct STRUCT_ITEMLIST
{
	char Name[64];
	short nIndexMesh;
	short nIndexTexture;
	short nIndexVisualEffect;
	short nReqLvl;
	short nReqStr;
	short nReqInt;
	short nReqDex;
	short nReqCon;
	STRUCT_STATICEFFECT stEffect[12];
	int nPrice;
	short nUnique;
	short UNK_1;
	int nPos;
	short nExtra;
	short nGrade;
	int UNK_2;
	short mType;
	short mData;
	short UNK_3;
	short UNK_4;
};

struct STRUCT_EXT1
{
	int Data[8];
	STRUCT_AFFECT Affect[32];
};

struct STRUCT_SUBCLASS
{
	unsigned int LearnedSkill[2];
	STRUCT_ITEM Equip;
	STRUCT_SCORE CurrentScore;
	long long Exp;
	char ShortSkill[20];
	char Reserved[4];
};

struct STRUCT_EXT2
{
	char Quest[12];
	unsigned int LastConnectTime;
	STRUCT_SUBCLASS SubClass[2];
	char ItemPassWord[16];
	unsigned int ItemPos;
	int SendLevItem;
	short AdminGuildItem;
	char Dummy[126];
};

struct STRUCT_DAM
{
	unsigned short TargetID;
	// WYD 7.48 stores each target as one WORD id plus one signed WORD damage.
	// Keeping this pair at four bytes restores AttackOne/Two/Multi to 48/52/96.
	short Damage;
};

struct STRUCT_SPELL
{
	int SkillPoint;
	int TargetType;
	int ManaSpent;
	int Delay;
	int Range;
	int InstanceType;
	int InstanceValue;
	int TickType;
	int TickValue;
	int AffectType;
	int AffectValue;
	int AffectTime;
	char Act1[8];
	char Act2[8];
	int InstanceAttribute;
	int TickAttribute;
	int Aggressive;
	int MaxTarget;
	int bParty;
	int AffectResist;
	int Passive;
	int ForceDamage;

	int UNK_01;
	int UNK_02;

};

struct STRUCT_AIRMOVELIST
{
	int nX;
	int nY;
};

struct STRUCT_TOTOLIST
{
	char szTime[32];
	char szTeamA[32];
	char szTeamB[32];
};

struct STRUCT_LOTTO
{
	short sIndex;
	char Num[6];
};

struct STRUCT_M_CHECK
{
	int Type;
	int Parm1;
	int Parm2;
};

struct STRUCT_GUILDLIST
{
	int GuildIndex;
	char GuildName[4][12];
	char GCount;
	char Citizen;
	char Mandle;
	unsigned int Fame;
};

struct STRUCT_TREASURE
{
	short Source;
	STRUCT_ITEM Target[5];
	short Rate[5];
};

struct STRUCT_TOTODATA
{
	int index;
	int A_Score;
	int B_Score;
	unsigned int A_BuyCount;
	unsigned int B_BuyCount;
	unsigned int C_BuyCount;
	int towshare;
};

struct STRUCT_ACCOUNT
{
	char AccountName[16];
	char AccountPass[12];

	char ItemPassWord[16];

	char RealName[24];
	unsigned int SSN1;
	unsigned int SSN2;
	char Temp[102];
	STRUCT_AFFECT Affect[2];
	unsigned int AccountIndex;
	unsigned int AccountLastConnectTime;

	unsigned short GameServer;
	unsigned short Rsv;
};

struct STRUCT_ACCOUNTFILE
{
	STRUCT_ACCOUNT Account;
	STRUCT_MOB Char[4];
	STRUCT_ITEM Cargo[MAX_CARGO];
	int Coin;
	char ShortSkill[4][16];
	STRUCT_EXT1 Ext1[4];
	STRUCT_EXT2 Ext2[4];
};

struct STRUCT_REQ
{
	bool Class;
	bool Level;
	bool Str;
	bool Dex;
	bool Int;
	bool Con;
};
struct LojaDonate
{
	int type;
	int page;
	STRUCT_ITEM item;
	int price;
	int stuck;
	int slot;

	LojaDonate(int type, int page, STRUCT_ITEM item, int price, int stuck, int slot)
	{
		this->type = type;
		this->page = page;
		memcpy_s(&this->item, sizeof(STRUCT_ITEM), &item, sizeof(STRUCT_ITEM));
		this->price = price;
		this->stuck = stuck;
		this->slot = slot;
	}
};

struct LojaDonateInfor
{
	int Type;

	int Page;

	int Slot;
};

struct STRUCT_LEVSENDITEM
{
	int lev;
	STRUCT_ITEM item[4][2];
};

struct STRUCT_RANDOMQUIZ
{
	char Question[128];
	char Answer[4][32];
};

struct
	STRUCT_ITEMHELP
{
	int Color[9];
	char Help[9][128];
};

struct STRUCT_COMBINE
{
	int Target;
	int Source[8];
	int Refine[8];
};

struct STRUCT_BEASTBONUS
{
	int Int_Damage;
	int Spe_Damage;
	int Int_Ac;
	int Spe_Ac;
	int Int_Hp;
	int Spe_Hp;
	int Int_Run;
	int Spe_Run;
	int Int_Attack;
	int Spe_Attack;
	int Int_Critical;
	int Spe_Critical;
	int Int_Resist;
	int Spe_Regist;
	int Int_Delay;
	int Spe_Delay;
	int SancLevel;
};

struct STRUCT_EXT
{
	int Data[8];
	STRUCT_AFFECT Affect[12];
	char Quest[32];
};

struct STRUCT_MISSION
{
	int Arrival;
	STRUCT_ITEM cItem[8];
	STRUCT_ITEM rItem[10];
	STRUCT_M_CHECK Item[8][10];
	STRUCT_M_CHECK Condition[10];
	STRUCT_M_CHECK Reward[10];
};

struct STRUCT_GUILDZONE
{
	int ChargeGuild;
	int ChallangeGuild;
	int GuildStartX;
	int GuildStartY;
	STRUCT_POSITION Start;
	int vx1;
	int vy1;
	int vx2;
	int vy2;
	int ax1;
	int ay1;
	int ax2;
	int ay2;
	int hx;
	int hy;
	int gx;
	int gy;
	int Tax;
	int ChargeClan;
	int ChargeCount;
};

struct STRUCT_ACCOUNT_NEW
{
	char AccountName[16];
	char AccountPass[12];
	char RealName[24];
	unsigned int SSN1;
	unsigned int SSN2;
	char Temp[102];
	STRUCT_AFFECT Affect[2];
	unsigned int AccountIndex;
	unsigned int AccountLastConnectTime;
	char ItemPassWord[16];
	unsigned short GameServer;
	unsigned short Rsv;
};

struct STRUCT_SAME
{
	short dINDEX[8];
	unsigned int dAttribute;
};

struct STRUCT_RARE
{
	int MonsterFace;
	int DropRate;
	STRUCT_ITEM item;
	int Count;
};

struct STRUCT_ADMINGUILDITEM
{
	int lev;
	int kind;
	STRUCT_ITEM item;
};

struct STRUCT_ACCOUNTFILE_NEW
{
	STRUCT_ACCOUNT_NEW Account;
	STRUCT_MOB Char[4];
	STRUCT_ITEM Cargo[MAX_CARGO];
	int Coin;
	char ShortSkill[4][16];
	STRUCT_EXT1 Ext1[4];
	STRUCT_EXT2 Ext2[4];
};

struct STRUCT_RUNEQUESTZONE
{
	int StartX;
	int StartY;
	int iLeader;
	int Type;
};

struct STRUCT_AUTOKICK
{
	char route[4][128];
};
