#pragma once

#include "ProcessClientMessage.h"
#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <random>
#include <ctime>
#include "SendFunc.h"
#include "Functions.h"

void FiltroDrop(int conn, int Index) {

	if (conn <= 0 || conn >= MAX_USER)
		return;

	if (pUser[conn].Mode != USER_PLAY)
		return;

	if (pUser[conn].cSock.Sock == 0)
		return;

	if (Index <= 0 || Index > MAX_ITEMLIST)
		return; 

	if (pUser[conn].FiltroDelay != 0)
	{
		int isTime = GetTickCount64() - pUser[conn].FiltroDelay;

		if (isTime < 600)
			return;

	}
	pUser[conn].FiltroDelay = GetTickCount64();

	if (pMob[conn].FiltroState == 0) {
		SendClientMessage(conn, "Ative o filtro para adicionar itens");
		return;
	}

	if (pMob[conn].MOB.Equip[13].sIndex != 3901 && pMob[conn].MOB.Equip[13].sIndex != 3902 && pMob[conn].MOB.Equip[13].sIndex != 3916) {
		SendClientMessage(conn, "Precisa de uma Fada Azul/Vermelha ou do vale para usar essa função!");
		return;
	} 

	for (int i = 0; i < 16; i++)
	{
		if (pMob[conn].FiltroID[i] == Index)
		{ 
			 
			SendClientMessage(conn, strFmt("[%s] Removido a lista do filtro", g_pItemList[pMob[conn].FiltroID[i]].Name));

			pMob[conn].FiltroID[i] = 0;
			SendFilterList(conn);
			return;
		}

	}

	for (int i = 0; i < 16; i++) 
	{
		if (pMob[conn].FiltroID[i] == 0)
		{
			pMob[conn].FiltroID[i] = Index;
			SendClientMessage(conn, strFmt("[%s] Adicionado a lista do filtro", g_pItemList[pMob[conn].FiltroID[i]].Name));

			SendFilterList(conn);
			return;
		}

	} 
	 
}

void SendFilterList(int conn) {

	if (conn <= 0 || conn >= MAX_USER)
		return;

	if (pUser[conn].Mode != USER_PLAY)
		return;

	if (pUser[conn].cSock.Sock == 0)
		return;

	MSG_UpdateFilterList sm;
	memset(&sm, 0, sizeof(MSG_UpdateFilterList));
	sm.Type = _MSG_UpdateFilterList;
	sm.Size = sizeof(MSG_UpdateFilterList);
	sm.ID = conn;
	for (int i = 0; i < 16; i++)//limite atual do jogo!
	{
		if(pMob[conn].FiltroID[i])
		{
			sm.Item[i] = pMob[conn].FiltroID[i];
		}
	}
	 
	if (!pUser[conn].cSock.AddMessage((char*)&sm, sizeof(MSG_UpdateFilterList)))
		CloseUser(conn);

}