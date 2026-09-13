/*
*   Copyright (C) {2015}  {Victor Klafke, Charles TheHouse}
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see [http://www.gnu.org/licenses/].
*
*   Contact at: victor.klafke@ecomp.ufsm.br
*/
#include "ProcessClientMessage.h"

void Exec_MSG_SendAutoTrade(int conn, char *pMsg)
{
	MSG_SendAutoTrade* m = (MSG_SendAutoTrade*)pMsg;

	int Size = m->Size;

	for (int i = 0; i < MAX_AUTOTRADE; i++) {
		if (m->Coin[i] < 0 || m->Coin[i] > 2000000000) {
			RemoveTrade(conn);
			return;
		}
	}

	if (m->Tax < 0) {
		RemoveTrade(conn);
		return;
	}

	if (m->Index < 0) {
		RemoveTrade(conn);
		return;
	}

	if (Size > sizeof(MSG_SendAutoTrade)) //CONTROLE DE SIZE
	{
		SendClientMessage(conn, "Impossível executar ação47, tente mais tarde. ");
		return;
	}

	if (pMob[conn].MOB.CurrentScore.Hp <= 0 || pUser[conn].Mode != USER_PLAY)
	{
		SendHpMode(conn);
		AddCrackError(conn, 10, 88);
		RemoveTrade(conn);
		return;
	}

	if (pUser[conn].Trade.OpponentID)
	{
		RemoveTrade(pUser[conn].Trade.OpponentID);
		RemoveTrade(conn);
		return;
	}


	if (pUser[conn].Ingame.Lojinha == 1) {
		DeleteMob(25000 + conn, 2);
		pUser[conn].Ingame.Lojinha = 0;
		RemoveTrade(conn);
		SendClientMessage(conn, "Sua loja atual irá desaparecer em alguns segundos");
		return;
	}

	if (Limitadordelojinha(pUser[conn].MacAddress) >= 5)
	{
		SendClientMessage(conn, "Apenas 1 AutoLoja por máquina.");
		RemoveTrade(conn);
		return;
	}
	///////////////////////////////////////////////
	//Limitador de Lojinhas por IP
	int Connections = 0;
	for (int z = 0; z < MAX_USER; z++)
	{
		if (pUser[z].Mode != USER_PLAY)
			continue;

		if (!pUser[z].cSock.Sock)
			continue;

		if (pUser[z].Ingame.Lojinha == 5)
		{
			if (pUser[z].IP == pUser[conn].IP)
				Connections++;
		}
	}
	if (Connections >= 5) {
		SendClientMessage(conn, "Apenas 1 Autoloja por IP");
		RemoveTrade(conn);
		return;
	}

	if (pUser[conn].TradeMode)
	{
		SendClientMessage(conn, g_pMessageStringTable[_NN_CantWhenAutoTrade]);
		return;
	}

	if (NewbieEventServer == 0)
	{
		SendClientMessage(conn, g_pMessageStringTable[_NN_NEWBIEEVENTSERVER]);
		return;
	}

	int targetx = pMob[conn].TargetX;
	int targety = pMob[conn].TargetY;

	int Village = BASE_GetVillage(targetx, targety);

	if (targetx >= 2123 && targetx <= 2148 && targety >= 2139 && targety <= 2157)
	{
		RemoveTrade(conn);
		SendClientMessage(conn, g_pMessageStringTable[_NN_OnlyVillage]);
		return;
	}

	if (Village < 0 || Village >= 5)
	{
		RemoveTrade(conn);
		SendClientMessage(conn, g_pMessageStringTable[_NN_OnlyVillage]);
		return;
	}

	if (conn <= 0 || conn >= MAX_USER)
		return;

	if (m->Item[0].sIndex == 0) {
		SendClientMessage(conn, "Necessário pelo menos um item");
		return;
	}

	for (int i = 0; i < MAX_AUTOTRADE; i++)
	{
		if (m->Coin[i] < 0 || (m->Coin[i] > 1999999999) || !m->Item[i].sIndex && m->Coin[i])
			return;

		if (m->Item[i].sIndex)
		{
			if (!m->Coin[i] || m->Item[i].sIndex == 508 || m->Item[i].sIndex == 3993 || m->Item[i].sIndex == 747 || m->Item[i].sIndex == 509 || m->Item[i].sIndex == 522
				|| m->Item[i].sIndex >= 526 && m->Item[i].sIndex <= 531 || m->Item[i].sIndex == 446 || m->CarryPos[i] < 0 || m->CarryPos[i] >= MAX_CARGO)
				return;

			if (BASE_GetItemAbility(&m->Item[i], EF_NOTRADE) != 0)
			{
				SendClientMessage(conn, g_pMessageStringTable[_NN_Cant_MoveItem]);
				return;
			}

			if (memcmp(&pUser[conn].Cargo[m->CarryPos[i]], &m->Item[i], sizeof(STRUCT_ITEM)))
			{
				RemoveTrade(conn);
				return;
			}
		}
	}

	memcpy(&pUser[conn].AutoTrade, m, sizeof(MSG_SendAutoTrade));

	SendClientMessage(conn, "Você irá receber uma recompensa a cada 10 minutos!");
	pMob[conn].CheckLojinha = 600;
	pUser[conn].Ingame.Lojinha = 1;
	RemoveTrade2(conn);
	CreateMobStore("AutoStore", pMob[conn].TargetX, pMob[conn].TargetY, "npc", 0, (MAX_MOB + conn), conn);
	return;
}