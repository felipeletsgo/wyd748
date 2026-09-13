#pragma once

#include <stdio.h>
#include "..\Basedef.h"

extern int BattlePassPremium;
extern int BattlePassStatus;

#define LIMITE_PARTIC 10
#define MAX_ARENAS 8

struct JOGADORES {
	int Player[4][LIMITE_PARTIC];
	int Equipe;
	int Kills[4];
};

struct ARENAREAL {
	JOGADORES Jogadores;
	STRUCT_ITEM ItemPremio;
	STRUCT_ITEM ItemConsolo;
	STRUCT_ITEM ItemRegistro;
	int GoldRegistro;
	int EvoMin;
	int LevelMin;
	int HonraPremio;
	int HonraConsolo;
	int Hora[MAX_ARENAS];
	int Minuto[MAX_ARENAS];
	int Time[2];
	int State;
	int Status;
};


extern ARENAREAL ArenaReal;
extern int ARTeleport[10][2];
extern int GrupoPos[4][2];

void ArenaRealTimer();
void ArenaRealFinaliza();
void StartArenaReal();
void RegistroArenaReal(int conn);
void UpdateArenaRealUI();
void KillArenaReal(int killer, int killed);

void UpdateDropList(int conn, const char* mobname, int region);
void FiltroDrop(int conn, int Index);
void ReqDropList(int conn, char* pMsg);
void ReqdListNames(int conn, char* pMsg);
void ReqTeleport(int conn, char* pMsg);
void ReqDonateShop(int conn, char* pMsg);
void EventoClick(int conn, int npc);
void Jephi(int conn, char* pMsg);
void ItemLevel(int conn, char* pMsg);
void TradeDonate(int conn, char* pMsg);
void UpdateShopDonate1();
void UpdateShopDonate2();
void UpdateShopDonate3();
void UpdateShopDonate4();
void UpdateShopDonate5();
void UpdateShopDonate6();
void UpdateShopDonate7();
void UpdateShopDonate8();
void UpdateShopDonate9();
void UpdateShopDonate10(); 
void SendShopDonate(int conn);
void RequestAlias(int conn, char* pMsg);
void MailItem(int conn);
void SendMailItem(int conn);
void doRanking(int conn);
void sendRanking(int conn, int state);
void GerarPix(int conn);
void GerarRecaptcha();
void VerificarRecaptcha(int conn, int word);
void SendRecaptcha(int conn, int territorio);

void attMountTraje(int conn, char* pMsg);
void SetMountTraje(int conn, int slot);
void RemoveMountTraje(int conn);
bool MountTrajeBuy(int conn, int Index);
void RequestTraje(int conn);

int NumbGen(int Initial, int Final);
void LoadEvent();
void ReadServerConfigs();
void ReadExpTableN();
void ReadExpTableM();
//void ReadExpTableA();
void ReqNPC(int conn, char* pMsg);
void DoComposition(int conn, int NPCID, int COMPID);
void ReadNewCompsConfig();
void ReadNewComps();

int ControleDropGold(int conn, int target, int DropedGold);
STRUCT_ITEM* ControleDropItem(int conn, int target, int bonus, int PosX, int PosY, STRUCT_ITEM* Item);
void LanDrop(int conn, int target);
void CartaDrop(int conn, int target, int type);
bool AutoDrop(int conn, int itemId);
bool AgroupList(int conn, int itemId);
bool LogList(int itemId);
void Freeze();
void BlockAdd(STRUCT_ITEM* item, int Type, int min, int max);
bool DropItemControl(int sIndex);
void MacroNextSala(int conn, int Type, int Sala);

bool ChangeBPPoints(int conn, int amount, int state);
void ReloadBattlePassItens();
void ReloadBattlePassMissions();
void bPassMission(int conn, int mission);
void BattlePassReward(int conn);
void SendBattlePass(int conn, int page);
void BattlePassMobKill(int conn, int target);