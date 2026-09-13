#pragma once

#include "ProcessClientMessage.h"
#include "..\SecureString.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <random>
#include <ctime>
#include "SendFunc.h"
#include "Functions.h"
#include "wMySQL.h"

#define BPMOUNTH 45
#define BPLEVEL 47
#define BPPOINTS 48
#define MAXPOINTS 100000
#define MAXBPLEVEL 30
#define SUB 0
#define ADD 1
#define SET 2
#define MOUNTH 9 //9 = september

int BattlePassPremium;
int BattlePassStatus = 1;

bool ChangeBPPoints(int conn, int amount, int state) {

	if (conn <= 0 || conn >= MAX_USER)
		return false;

	if (pUser[conn].Mode != USER_PLAY)
		return false;

	if (pUser[conn].cSock.Sock == 0)
		return false;

	if (BattlePassStatus == 0)
		return false;

	if (state == 0) {
		if (pUser[conn].Keys[BPPOINTS] < amount)
			return false;

		pUser[conn].Keys[BPPOINTS] -= amount;
	}
	if (state == 1) {
		if (pUser[conn].Keys[BPMOUNTH] != MOUNTH) {
			pUser[conn].Keys[BPMOUNTH] = MOUNTH;
			pUser[conn].Keys[BPPOINTS] = 0;
			pUser[conn].Keys[BPLEVEL] = 0;
			return false;
		}

		if (pUser[conn].Keys[BPPOINTS] == MAXPOINTS)
			return false;

		if (pUser[conn].Keys[BPPOINTS] < MAXPOINTS) {
			if (pUser[conn].Keys[BPPOINTS] + amount >= MAXPOINTS)
				pUser[conn].Keys[BPPOINTS] = MAXPOINTS;
			else
				pUser[conn].Keys[BPPOINTS] += amount;
		}
	}
	if (state == 2)
		pUser[conn].Keys[BPPOINTS] = amount;

	return true;
}

void ReloadBattlePassItens() {

	auto& pc = cSQL::instance();

	SNPRINTF_SAFE(hQuery, sizeof(hQuery), "SELECT * FROM `battlepassitens` LIMIT 50");
	MYSQL_ROW row;
	MYSQL* wSQL = pc.wStart();
	MYSQL_RES* result = pc.wRes(wSQL, hQuery);

	if (result == NULL)
		return;

	int i = 0;

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		BattlePassItens[i].sIndex = atoi(row[2]);
		BattlePassItens[i].stEffect[0].cEffect = atoi(row[3]);
		BattlePassItens[i].stEffect[0].cValue = atoi(row[4]);
		BattlePassItens[i].stEffect[1].cEffect = atoi(row[5]);
		BattlePassItens[i].stEffect[1].cValue = atoi(row[6]);
		BattlePassItens[i].stEffect[2].cEffect = atoi(row[7]);
		BattlePassItens[i].stEffect[2].cValue = atoi(row[8]);
		i++;
	}
	mysql_free_result(result);
}

void ReloadBattlePassMissions() {

	auto& pc = cSQL::instance();

	SNPRINTF_SAFE(hQuery, sizeof(hQuery), "SELECT * FROM `bpassmissions` LIMIT 50");
	MYSQL_ROW row;
	MYSQL* wSQL = pc.wStart();
	MYSQL_RES* result = pc.wRes(wSQL, hQuery);

	if (result == NULL)
		return;

	int i = 0;

	while ((row = mysql_fetch_row(result)) != NULL)
	{
		bPassMissionsPoints[i] = atoi(row[2]);
		i++;
	}
	mysql_free_result(result);
}

void bPassMission(int conn, int mission) {

	if (BattlePassStatus == 0)
		return;

	ChangeBPPoints(conn, bPassMissionsPoints[mission], 1);
	return;
}

void BattlePassReward(int conn) {

	if (conn <= 0 || conn >= MAX_USER)
		return;

	if (pUser[conn].Mode != USER_PLAY)
		return;

	if (pUser[conn].cSock.Sock == 0)
		return;

	if (BattlePassStatus == 0)
		return;

	if (pUser[conn].Keys[BPLEVEL] == MAXBPLEVEL)
		return;

	if (BattlePassPremium && pUser[conn].Keys[46] == 0) {
		SendClientMessage(conn, "Voc� Precisa adquirir o Passe de Batalha");
		return;
	}

	if (pUser[conn].RewBattlePass != 0)
	{
		int isTime = GetTickCount64() - pUser[conn].RewBattlePass; // ATRASO PACOTE

		if (isTime < 400)
		{
			return;
		}

	}
	pUser[conn].RewBattlePass = GetTickCount64();

	if (pUser[conn].Keys[BPMOUNTH] != MOUNTH) {
		pUser[conn].Keys[BPMOUNTH] = MOUNTH;
		pUser[conn].Keys[BPPOINTS] = 0;
		pUser[conn].Keys[BPLEVEL] = 0;
		return;
	}

	if (pUser[conn].Keys[BPPOINTS] == MAXPOINTS) {
		int Level = pUser[conn].Keys[BPLEVEL];

		if (PutItem(conn, &BattlePassItens[Level]) == true) {
			pUser[conn].Keys[BPLEVEL]++;
			ChangeBPPoints(conn, 0, SET);
			int Page = (int)(Level / 10);
			SendBattlePass(conn, Page);
		}
	}
	else {
		char Prc[2] = { "%" };
		SendClientMessage(conn, strFmt("Your BattlePass Bar need to be at 100%s", Prc));
	}
}

void SendBattlePass(int conn, int page) {

	if (BattlePassStatus == 0)
		return;

	if (page > 5 || page < 0)
		return;

	if (pUser[conn].ReqBattlePass != 0)
	{
		int isTime = GetTickCount64() - pUser[conn].ReqBattlePass; // ATRASO PACOTE

		if (isTime < 400)
		{
			return;
		}

	}
	pUser[conn].ReqBattlePass = GetTickCount64();


	MSG_SendBattlePass sm;
	memset(&sm, 0, sizeof(MSG_SendBattlePass));

	sm.Type = _MSG_SendBattlePass;
	sm.Size = sizeof(MSG_SendBattlePass);
	sm.ID = conn;
	sm.Points = pUser[conn].Keys[BPPOINTS];
	sm.Level = pUser[conn].Keys[BPLEVEL];
	sm.Page = page;

	int Num = page * 10;

	for (int i = 0; i < 10; i++) {
		memcpy(&sm.Item[i], &BattlePassItens[i + Num], sizeof(STRUCT_ITEM));
	}

	int Size = sm.Size;

	if (Size > sizeof(MSG_SendBattlePass))
	{
		sm.Size = 0;
		return;
	}

	if (!pUser[conn].cSock.AddMessage((char*)&sm, sizeof(MSG_SendBattlePass)))
		CloseUser(conn);
}

void BattlePassMobKill(int conn, int target) {

	if (BattlePassStatus == 0)
		return;

	if (conn <= 0 || conn >= MAX_USER)
		return;

	if (pUser[conn].Mode != USER_PLAY)
		return;

	if (pUser[conn].cSock.Sock == 0)
		return;

	if (target < MAX_USER || target > MAX_MOB)
		return;

	std::string MobName = pMob[target].MOB.MobName;

	bPassMission(conn, 12);


	if (MobName == "Tauron") {
		bPassMission(conn, 13);
		return;
	}
	//LAN N
	if (MobName == "MorlockAmald" || MobName == "GargulaAmald" || MobName == "HorizonAmald" || MobName == "AranhaAmald") {
		bPassMission(conn, 14);
		return;
	}
	//LAN M
	if (MobName == "KalintzAmaldM" || MobName == "KalintzAmaldW" || MobName == "LoboAmald" || MobName == "EntAmald") {
		bPassMission(conn, 15);
		return;
	}
	//LAN A
	if (MobName == "xxxxx" || MobName == "xxxxx" || MobName == "xxxxx" || MobName == "xxxxx") {
		bPassMission(conn, 16);
		return;
	}
	// Vale
	if (MobName == "xxxxx" || MobName == "xxxxx" || MobName == "xxxxx" || MobName == "xxxxx") {
		bPassMission(conn, 17);
		return;
	}
}