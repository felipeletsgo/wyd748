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
#include "wMySQL.h"

//Composition Structures

#define NCOMPS 16 //Number of compositions
#define NITENS 8 //Number of itens we can ask
#define SITENS 7 //Structures of the item. sIndex + aff1, aff2, aff3
#define NNPCS 10 //Number of NPCs

int ViewItens[NNPCS][NCOMPS][SITENS];
int CompItens[NNPCS][NCOMPS][NITENS][SITENS];
int GoldRequest[NNPCS][NCOMPS];
int Rate[NNPCS][NCOMPS];

void ReqNPC(int conn, char* pMsg) {

	if (conn <= 0 || conn >= MAX_USER)
		return;

	if (pUser[conn].Mode != USER_PLAY)
		return;

	if (pUser[conn].cSock.Sock == 0)
		return;

	MSG_ReqNPC* m = (MSG_ReqNPC*)pMsg;

	int Size = m->Size;

	if (Size > sizeof(MSG_ReqNPC))
		return;

	if (m->NPCID < 0 || m->NPCID > NNPCS)
		return;

	if (m->State < 0 || m->State > 2)
		return;

	if (m->Comp < 0 || m->Comp > NCOMPS)
		return;

	if (pUser[conn].ReqComposer != 0)
	{
		int isTime = GetTickCount64() - pUser[conn].ReqComposer; // ATRASO PACOTE

		if (isTime < 800)
		{
			return;
		}

	}
	pUser[conn].ReqComposer = GetTickCount64();

	if (m->State == 2) {
		DoComposition(conn, m->NPCID, m->Comp);
		return;
	}

	MSG_SendNPC sm;
	memset(&sm, 0, sizeof(MSG_SendNPC));

	sm.Type = _MSG_SendNPC;
	sm.Size = sizeof(MSG_SendNPC);
	sm.ID = conn;
	sm.State = m->State;

	if (m->State == 0) {

		for (int i = 0; i < NCOMPS; i++) {

			if (ViewItens[m->NPCID][i][0] == 0)
				continue;

			sm.Item[i].sIndex = ViewItens[m->NPCID][i][0];
			sm.Item[i].stEffect[0].cEffect = ViewItens[m->NPCID][i][1];
			sm.Item[i].stEffect[0].cValue = ViewItens[m->NPCID][i][2];
			sm.Item[i].stEffect[1].cEffect = ViewItens[m->NPCID][i][3];
			sm.Item[i].stEffect[1].cValue = ViewItens[m->NPCID][i][4];
			sm.Item[i].stEffect[2].cEffect = ViewItens[m->NPCID][i][5];
			sm.Item[i].stEffect[2].cValue = ViewItens[m->NPCID][i][6];
		}
	}
	if (m->State == 1) {
		for (int i = 0; i < NITENS; i++) {

			if (CompItens[m->NPCID][m->Comp][i][0] == 0)
				continue;

			sm.Item[i].sIndex = CompItens[m->NPCID][m->Comp][i][0];
			sm.Item[i].stEffect[0].cEffect = CompItens[m->NPCID][m->Comp][i][1];
			sm.Item[i].stEffect[0].cValue = CompItens[m->NPCID][m->Comp][i][2];
			sm.Item[i].stEffect[1].cEffect = CompItens[m->NPCID][m->Comp][i][3];
			sm.Item[i].stEffect[1].cValue = CompItens[m->NPCID][m->Comp][i][4];
			sm.Item[i].stEffect[2].cEffect = CompItens[m->NPCID][m->Comp][i][5];
			sm.Item[i].stEffect[2].cValue = CompItens[m->NPCID][m->Comp][i][6];
			sm.Gold[0] = GoldRequest[m->NPCID][m->Comp];
			sm.Gold[1] = Rate[m->NPCID][m->Comp];
		}
	}

	int Size2 = sm.Size;

	if (Size2 > sizeof(MSG_SendNPC))
	{
		sm.Size = 0;
		return;
	}

	if (!pUser[conn].cSock.AddMessage((char*)&sm, sizeof(MSG_SendNPC)))
		CloseUser(conn);
}

void DoComposition(int conn, int NPCID, int COMPID) {

	if (ViewItens[NPCID][COMPID][0] == 0) {
		SendClientMessage(conn, "Composi��o Inv�lida");
		return;
	}

	if (pUser[conn].DoComposer != 0)
	{
		int isTime = GetTickCount64() - pUser[conn].DoComposer; // ATRASO PACOTE

		if (isTime < 1000)
		{
			return;
		}

	}
	pUser[conn].DoComposer = GetTickCount64();

	bool CanDoIt = true;
	int Slot[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };

	for (int i = 0; i < NITENS; i++) {

		if (CompItens[NPCID][COMPID][i][0] == 0)
			continue;

		int Need = 0;

		if (CompItens[NPCID][COMPID][i][1] != 61) {
			STRUCT_ITEM Item{ 0 };
			Item.sIndex = CompItens[NPCID][COMPID][i][0];
			Item.stEffect[0].cEffect = CompItens[NPCID][COMPID][i][1];
			Item.stEffect[0].cValue = CompItens[NPCID][COMPID][i][2];
			Item.stEffect[1].cEffect = CompItens[NPCID][COMPID][i][3];
			Item.stEffect[1].cValue = CompItens[NPCID][COMPID][i][4];
			Item.stEffect[2].cEffect = CompItens[NPCID][COMPID][i][5];
			Item.stEffect[2].cValue = CompItens[NPCID][COMPID][i][6];

			///////////////////////////// CHECKING THE ITENS
			for (int x = 0; x < MAX_CARRY - 1; x++)
			{
				if (pMob[conn].MOB.Carry[x].sIndex == 0)
					continue;

				bool Continue = false;

				for (int z = 0; z < NITENS; z++) {
					if (z == -1)
						continue;

					if (x == Slot[z]) {
						Continue = true;
						break;
					}
				}

				if (Continue)
					continue;

				if (memcmp(&pMob[conn].MOB.Carry[x], &Item, sizeof(STRUCT_ITEM)))
					continue;

				Slot[i] = x;
			}
			///////////////////////////

			if (Slot[i] == -1) {
				SendClientMessage(conn, "Voc� n�o possui todos os itens necess�rios");
				return;
			}
		}
		else {
			if (CompItens[NPCID][COMPID][i][1] == 61)
				Need += CompItens[NPCID][COMPID][i][2];
			if (CompItens[NPCID][COMPID][i][3] == 61)
				Need += CompItens[NPCID][COMPID][i][4];
			if (CompItens[NPCID][COMPID][i][5] == 61)
				Need += CompItens[NPCID][COMPID][i][6];

			if (Need == 0)
				Need = 1;

			if (GetItemCount(conn, CompItens[NPCID][COMPID][i][0]) < Need) {
				CanDoIt = false;
				break;
			}
		}
	}

	if (CanDoIt == false) {
		SendClientMessage(conn, "Voc� n�o possui todos os itens necess�rios");
		return;
	}

	if (GetFreeSlot(conn) == -1) {
		SendClientMessage(conn, "N�o h� espa�o suficiente no seu invent�rio");
		return;
	}

	if (pMob[conn].MOB.Coin < GoldRequest[NPCID][COMPID]) {
		SendClientMessage(conn, "Gold Insuficiente");
		return;
	}

	pMob[conn].MOB.Coin -= GoldRequest[NPCID][COMPID];

	for (int z = 0; z < NITENS; z++) {

		if (CompItens[NPCID][COMPID][z][0] == 0)
			continue;

		int Need = 0;

		if (CompItens[NPCID][COMPID][z][1] != 61) {
			if (Slot[z] != -1) {
				STRUCT_ITEM Item{ 0 };
				memcpy(&pMob[conn].MOB.Carry[Slot[z]], &Item, sizeof(STRUCT_ITEM));
				SendItem(conn, ITEM_PLACE_CARRY, Slot[z], &pMob[conn].MOB.Carry[Slot[z]]);
				SendCarry(conn);
			}
		}
		else
		{
			if (CompItens[NPCID][COMPID][z][1] == 61)
				Need += CompItens[NPCID][COMPID][z][2];
			if (CompItens[NPCID][COMPID][z][3] == 61)
				Need += CompItens[NPCID][COMPID][z][4];
			if (CompItens[NPCID][COMPID][z][5] == 61)
				Need += CompItens[NPCID][COMPID][z][6];

			if (Need == 0)
				Need = 1;

			RemoveItem(conn, CompItens[NPCID][COMPID][z][0], Need);
		}
	}

	int sRand = rand() % 100;

	if (sRand > Rate[NPCID][COMPID]) {
		SendClientMessage(conn, strFmt("Composi��o Falhou! %d/%d", sRand, Rate[NPCID][COMPID]));
		Combinelogsitem(pUser[conn].AccountName, pUser[conn].MacAddress, pUser[conn].IP, strFmt("NewCombine Failure! NPCID: %d and COMPID: %d", NPCID, COMPID));
		return;
	}

	STRUCT_ITEM Item{ 0 };
	Item.sIndex = ViewItens[NPCID][COMPID][0];
	Item.stEffect[0].cEffect = ViewItens[NPCID][COMPID][1];
	Item.stEffect[0].cValue = ViewItens[NPCID][COMPID][2];
	Item.stEffect[1].cEffect = ViewItens[NPCID][COMPID][3];
	Item.stEffect[1].cValue = ViewItens[NPCID][COMPID][4];
	Item.stEffect[2].cEffect = ViewItens[NPCID][COMPID][5];
	Item.stEffect[2].cValue = ViewItens[NPCID][COMPID][6];

	if (PutItem(conn, &Item) == false) {
		Combinelogsitem(pUser[conn].AccountName, pUser[conn].MacAddress, pUser[conn].IP, strFmt("Delivered Failure! NPCID: %d and COMPID: %d", NPCID, COMPID));
	}
	SendCarry(conn);
	SaveUser(conn, 0);
	SendClientMessage(conn, strFmt("Sucesso! %d/%d", sRand, Rate[NPCID][COMPID]));
	Combinelogsitem(pUser[conn].AccountName, pUser[conn].MacAddress, pUser[conn].IP, strFmt("Composition success! NPCID: %d and COMPID: %d", NPCID, COMPID));
}


void ReadNewCompsConfig() {

	auto& pc = cSQL::instance();

	SNPRINTF_SAFE(hQuery, sizeof(hQuery), "SELECT * FROM `ncompositorinfo`");
	MYSQL_ROW row;
	MYSQL* wSQL = pc.wStart();
	MYSQL_RES* result = pc.wRes(wSQL, hQuery);

	if (result == NULL)
		return;

	for (int i = 0; i < NNPCS; i++) {
		for (int c = 0; c < NCOMPS; c++) {
			for (int z = 0; z < 7; z++)
				ViewItens[i][c][z] = 0;
		}
	}

	while ((row = mysql_fetch_row(result)) != NULL) {
		int npc = atoi(row[1]);
		int comp = atoi(row[2]);

		if (npc >= NNPCS || comp >= NCOMPS)
			continue;

		Rate[npc][comp] = atoi(row[3]);
		GoldRequest[npc][comp] = atoi(row[4]);

		ViewItens[npc][comp][0] = atoi(row[5]);
		ViewItens[npc][comp][1] = atoi(row[6]);
		ViewItens[npc][comp][2] = atoi(row[7]);
		ViewItens[npc][comp][3] = atoi(row[8]);
		ViewItens[npc][comp][4] = atoi(row[9]);
		ViewItens[npc][comp][5] = atoi(row[10]);
		ViewItens[npc][comp][6] = atoi(row[11]);
	}
	mysql_free_result(result);
}

void ReadNewComps() {

	auto& pc = cSQL::instance();

	SNPRINTF_SAFE(hQuery, sizeof(hQuery), "SELECT * FROM `newcompositor`");
	MYSQL_ROW row;
	MYSQL* wSQL = pc.wStart();
	MYSQL_RES* result = pc.wRes(wSQL, hQuery);

	if (result == NULL)
		return;

	for (int i = 0; i < NNPCS; i++) {
		for (int c = 0; c < NCOMPS; c++) {
			for (int s = 0; s < NITENS; s++) {
				for (int z = 0; z < 7; z++)
					CompItens[i][c][s][z] = 0;
			}
		}
	}

	while ((row = mysql_fetch_row(result)) != NULL) {
		int npc = atoi(row[1]);
		int comp = atoi(row[2]);
		int nitem = atoi(row[3]);

		if (npc >= NNPCS || comp >= NCOMPS || nitem >= NITENS)
			continue;

		CompItens[npc][comp][nitem][0] = atoi(row[4]);
		CompItens[npc][comp][nitem][1] = atoi(row[5]);
		CompItens[npc][comp][nitem][2] = atoi(row[6]);
		CompItens[npc][comp][nitem][3] = atoi(row[7]);
		CompItens[npc][comp][nitem][4] = atoi(row[8]);
		CompItens[npc][comp][nitem][5] = atoi(row[9]);
		CompItens[npc][comp][nitem][6] = atoi(row[10]);
	}
	mysql_free_result(result);
}