#pragma once

#include "ProcessClientMessage.h"
#include "..\SecureString.h"
#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <random>
#include <ctime>
#include "SendFunc.h"
#include "Functions.h"
#include "Chests.h"
#include "wMySQL.h"

#define GUARDA_REAL_I			35
#define GUARDA_REAL_II  		36
#define GUARDA_REAL_III 		37
#define GUARDA_REAL_IV  		38
#define GUARDA_REAL_V   		39
#define GUARDA_REAL_VI  		40

int NumbGen(int Initial, int Final) {
	static std::mt19937 rng(std::random_device{}());
	static std::uniform_int_distribution<int> dist(Initial, Final);
	return dist(rng);
}

void LoadEvent() {

	auto& pc = cSQL::instance();

	SNPRINTF_SAFE(xQuery, sizeof(xQuery), "SELECT * FROM `evento`");
	MYSQL_ROW row;
	MYSQL* wSQL = pc.wStart();
	MYSQL_RES* result = pc.wRes(wSQL, xQuery);

	if (result == NULL)
	{
		return;
	}
	int i = 0;
	while ((row = mysql_fetch_row(result)) != NULL)
	{
		if (i == 6)
			i = 0;

		int npc = atoi(row[2]);
		Evento[npc][i].sIndex = atoi(row[4]);
		Evento[npc][i].stEffect[0].cEffect = atoi(row[5]);
		Evento[npc][i].stEffect[0].cValue = atoi(row[6]);
		Evento[npc][i].stEffect[1].cEffect = atoi(row[7]);
		Evento[npc][i].stEffect[1].cValue = atoi(row[8]);
		Evento[npc][i].stEffect[2].cEffect = atoi(row[9]);
		Evento[npc][i].stEffect[2].cValue = atoi(row[10]);
		i++;
	}
	mysql_free_result(result);

	auto& pc2 = cSQL::instance();

	SNPRINTF_SAFE(xQuery, sizeof(xQuery), "SELECT * FROM `eventoconfig`");
	MYSQL_ROW row2;
	MYSQL* wSQL2 = pc2.wStart();
	MYSQL_RES* result2 = pc2.wRes(wSQL2, xQuery);

	if (result2 == NULL)
	{
		return;
	}
	while ((row2 = mysql_fetch_row(result2)) != NULL)
	{
		int Npc = atoi(row2[2]);
		int Item = atoi(row2[4]);
		BausIndex[Npc][Item] = atoi(row2[3]);
	}
	mysql_free_result(result2);
}


void EventoClick(int conn, int npc)
{
	if (conn <= 0 || conn >= MAX_USER)
		return;

	if (pUser[conn].Mode != USER_PLAY)
		return;

	if (pUser[conn].cSock.Sock == 0)
		return;

	int NPC = npc;
	int npcid = 0;
	bool Check = false;

	if (NPC == GUARDA_REAL_I) {
		npcid = 0;
		Check = true;
	}
	if (NPC == GUARDA_REAL_II) {
		npcid = 1;
		Check = true;
	}
	if (NPC == GUARDA_REAL_III) {
		npcid = 2;
		Check = true;
	}
	if (NPC == GUARDA_REAL_IV) {
		npcid = 3;
		Check = true;
	}
	if (NPC == GUARDA_REAL_V) {
		npcid = 4;
		Check = true;
	}
	if (NPC == GUARDA_REAL_VI) {
		npcid = 5;
		Check = true;
	}
	/*if (NPC == GUARDA_REAL_VII) {
		npcid = 6;
		Check = true;
	}
	if (NPC == GUARDA_REAL_VIII) {
		npcid = 7;
		Check = true;
	}*/

	if (Check == true) {
		int FreeSlot = 0;
		int slotsInv = 30;

		if (pMob[conn].MOB.Carry[60].sIndex == 3467)
			slotsInv += 15;
		if (pMob[conn].MOB.Carry[61].sIndex == 3467)
			slotsInv += 15;

		int itemIndex[3] = {};
		int ItemCount[3] = {};

		for (int o = 0; o < 3; o++) {
			itemIndex[o] = BausIndex[npcid][o];
			ItemCount[o] = 0;
		}

		for (int y = 0; y < 3; y++) {
			if (itemIndex[y] != 0) {

				for (int i = 0; i < slotsInv; i++) {
					if (pMob[conn].MOB.Carry[i].sIndex == itemIndex[y])
						ItemCount[y] += pMob[conn].MOB.Carry[i].stEffect[0].cEffect == 61 ? pMob[conn].MOB.Carry[i].stEffect[0].cValue : 1;

					if (pMob[conn].MOB.Carry[i].sIndex == 0)
						FreeSlot += 1;
				}
				if (ItemCount[y] < 1) {
					SendClientMessage(conn, strFmt("Voc� precisa de pelo menos 1 [%s] para fazer essa troca", g_pItemList[itemIndex[y]].Name));
					return;
				}
				if (FreeSlot < 1) {
					SendClientMessage(conn, "N�o h� espa�o dispon�vel no invent�rio");
					return;
				}
			}
		}

		for (int z = 0; z < 3; z++) {
			if (itemIndex[z] != 0) {
				RemoveItem(conn, itemIndex[z], 1);
			}
		}

		int sRand = 0;
		for (int x = 0; x < 6; x++)
			if (Evento[npcid][x].sIndex != 0)
				sRand++;

		// Verificar se há itens configurados para evitar divisão por zero
		if (sRand == 0) {
			SendClientMessage(conn, "Este NPC não tem recompensas disponíveis. Contate a administração.");
			return;
		}

		int rand_ = rand() % sRand;

		STRUCT_ITEM item;
		memset(&item, 0, sizeof(STRUCT_ITEM));

		memcpy(&item, &Evento[npcid][rand_], sizeof(STRUCT_ITEM));

		int Eff = item.stEffect[0].cEffect;
		int Qnt = item.stEffect[0].cValue;

		if (Eff == 61 && (pUser[conn].Keys[49] == 1 || pMob[conn].MOB.Equip[13].sIndex == 3902))
		{
			// Usar nova função que adiciona a quantidade correta
			SendItemagruparEx(conn, item.sIndex, Qnt);
		}
		else
			PutItem(conn, &item);

		SendMsgExp(conn, strFmt("Recebeu [%s]", g_pItemList[item.sIndex].Name), TNColor::CornBlue, false);
	}
}