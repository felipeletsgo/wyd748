#pragma once

// Declarações das tabelas e handles definidos em Basedef.cpp.
// Este fragmento é incluído ao final de Basedef.h, depois dos tipos legados;
// não deve ser incluído isoladamente porque seus tipos pertencem à fachada.
extern char g_pAffectTable[MAX_EFFECT_STRING_TABLE][24];
extern char g_pAffectSubTable[MAX_SUB_EFFECT_STRING_TABLE][24];
extern int g_pHitRate[1024];
extern HWND hWndMain;
extern char EncodeByte[4];
extern int g_nChannelWidth;
extern int g_nServerGroupNum;
extern char g_pMessageStringTable[MAX_STRING][MAX_STRING_LENGTH];
extern char g_pServerList[MAX_SERVERGROUP][MAX_SERVERNUMBER][64];
extern int g_nSelServerWeather;
extern STRUCT_ITEMLIST g_pItemList[MAX_ITEMLIST];
extern STRUCT_GUILDZONE g_pGuildZone[MAX_GUILDZONE];
extern STRUCT_SPELL g_pSpell[MAX_SPELL_LIST];
extern STRUCT_INITITEM g_pInitItem[100];
extern int g_itemicon[6500];
extern STRUCT_TOTOLIST g_pTOTOList[80];
extern int g_nTOTOListCount;
