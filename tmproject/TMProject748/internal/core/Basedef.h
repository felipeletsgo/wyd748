#pragma once

// Client base definitions. Types, packets and tables live in ordered
// chapters; this header adds the BASE_* functions defined in Basedef*.cpp.
#include "BasedefLimits.h"
#include "BasedefStructs.h"
#include "BasedefPackets.h"
#include "BasedefTables.h"

#include "BasedefGlobals.h"

float BASE_ScreenResize(float size);
char* strfmt(const char* str, ...);

void BASE_InitModuleDir();
void BASE_InitializeHitRate();
int BASE_InitializeAttribute();
int BASE_InitializeHeightMap();
void BASE_ApplyAttribute(char* pHeight, int size);
int BASE_ReadItemList();
int BASE_ReadSkillBin();
void BASE_InitialItemRePrice();
int	BASE_ReadMessageBin();
void BASE_InitEffectString();
int BASE_InitializeBaseDef();
void BASE_ReadItemPrice();
void BASE_UnderBarToSpace(char* szStr);
int BASE_InitializeServerList();
int	BASE_GetHttpRequest(char* httpname, char* Request, int MaxBuffer);
int BASE_GetSum(char* p, int size);
int BASE_GetSum2(char* p, int size);
int BASE_GetWeekNumber();
int BASE_GetItemSanc(STRUCT_ITEM* item);
int BASE_GetItemAbility(STRUCT_ITEM* item, char Type);
int BASE_DefineSkinMeshType(int nClass);
float BASE_GetMountScale(int nSkinMeshType, int nMeshIndex);
int BASE_GetLanguage();
int BASE_GetVillage(int x, int y);
int BASE_GetRoute(int x, int y, int* targetx, int* targety, char* Route, int distance, char* pHeight, int MH);
int BASE_GetDistance(int x1, int y1, int x2, int y2);
int BASE_GetSpeed(STRUCT_SCORE* score);
int BASE_GetSubGuild(int item);
unsigned int BASE_GetItemTenColor(STRUCT_ITEM* pItem);
int BASE_GetItemColorEffect(STRUCT_ITEM* item);
char BASE_CheckValidString(char* name);
char* BASE_TransCurse(char* sz);
char BASE_GetAttribute(int x, int y);
char BASE_GetAttr(int nX, int nY);
int BASE_ReadTOTOList(char* szFileName);
int BASE_GetStaticItemAbility(STRUCT_ITEM* item, char Type);
int BASE_IsInLowZone(int nX, int nY);
int BASE_GetItemAmount(STRUCT_ITEM* item);
int BASE_CanCarry(STRUCT_ITEM* Carry, int pos);
int BASE_CanTrade(STRUCT_ITEM* Dest, STRUCT_ITEM* Carry, char* MyTrade, STRUCT_ITEM* OpponentTrade);
void BASE_ClearItem(STRUCT_ITEM* item);
void BASE_SortTradeItem(STRUCT_ITEM* Item, int Type);
int BASE_CanCargo(STRUCT_ITEM* item, STRUCT_ITEM* cargo, int DestX, int DestY);
int BASE_CanEquip(STRUCT_ITEM* item, STRUCT_SCORE* score, int Pos, int Class, STRUCT_ITEM* pBaseEquip, int OriginalFace, bool hasSoulLimitSkill);
unsigned int BASE_GetItemColor(STRUCT_ITEM* item);
int BASE_GetColorCount(unsigned int dwColor);
int BASE_GetManaSpent(int SkillNumber, int SaveMana, int Special);
int BASE_GetSkillDamage(int dam, int ac, int combat);
int BASE_GetSkillDamage(int skillnum, STRUCT_MOB* mob, int weather, int weapondamage, int OriginalFace);
int BASE_CanEquip_RecvRes(STRUCT_REQ* req, STRUCT_ITEM* item, STRUCT_SCORE* score, int Pos, int Class, STRUCT_ITEM* pBaseEquip, int OriginalFace);
int BASE_GetBonusItemAbilityNosanc(STRUCT_ITEM* item, char Type);;
int BASE_GetBonusItemAbility(STRUCT_ITEM* item, char Type);
int BASE_GetItemAbilityNosanc(STRUCT_ITEM* item, char type);
unsigned int BASE_GetOptionColor(int nPos, unsigned int dwParam, int nValue);
void BASE_SetItemAmount(STRUCT_ITEM* item, int amount);
int BASE_GetMobAbility(STRUCT_MOB* mob, char Type);
int BASE_GetMaxAbility(STRUCT_MOB* mob, char Type);
char BASE_CheckChatValid(const char* Chat);
char CheckGuildName(const char* GuildName, bool bSubguild);
void BASE_GetHitPosition(int sx, int sy, int* tx, int* ty, char* pHeight, int MH);
int BASE_Get3DTo2DPos(float fX, float fY, float fZ, int* pX, int* pY);
int BASE_GetDoubleCritical(STRUCT_MOB* mob, unsigned short* sProgress, unsigned short* cProgress, char* bDoubleCritical);
void BASE_GetHitPosition2(int sx, int sy, int* tx, int* ty, char* pHeight, int MH);
void BASE_SetBit(char* byte, int pos);
int BASE_UpdateItem2(int maskidx, int CurrentState, int NextState, int xx, int yy, char* pHeight, int rotate, int height);
int BASE_GetMeshIndex(short sIndex);
bool BASE_CanRefine(STRUCT_ITEM* item);

int IsPassiveSkill(int nSkillIndex);

bool BASE_HasSancAdd(const STRUCT_BONUSEFFECT& effect);
int BASE_GetSancEffValue(const STRUCT_ITEM& item);

int BASE_GetItemSancSuccess(STRUCT_ITEM* item);

int BASE_GetEffectValue(STRUCT_ITEM* item, int effect);
void BASE_ChangeOrAddEffectValue(STRUCT_ITEM* item, int effect, int value);
void BASE_RemoveEffect(STRUCT_ITEM* item, int effect);

/* Read Functions */
void ReadItemName();
void ReadUIString();
char ReadNameFiltraDataBase();
char ReadChatFiltraDataBase();

/* String Related functions */
int IsClearString(char* str, int target);
int IsClearString2(char* str, int nTarget);

/* System functions */
void EnableSysKey();
bool CheckOS();
void DisableSysKey();

/* Other funtions */
int IsSkill(int nSkillIndex);
int GetSkillIndex(int nSkillIndex);
int IsValidSkill(int nSkillIndex);
int IsValidClassSkill(int nSkillIndex);
