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
#include "wMySQL.h"

void RequestAlias(int conn, char* pMsg)
{
	if (conn <= 0 || conn >= MAX_USER)
		return;

	if (pUser[conn].cSock.Sock == 0)
		return;

	MSG_ReqAlias* m = (MSG_ReqAlias*)pMsg;

	if (pUser[conn].Mode != USER_PLAY && m->Tipo != 29)
		return;

	int Size = m->Size;

	if (Size > sizeof(MSG_ReqAlias)) //CONTROLE DE SIZE
	{
		SendClientMessage(conn, "Impossível executar ação, tente mais tarde.");
		return;
	}


	if (m->Tipo < 0 || m->Tipo > 20000)
		return;

	int Type = m->Tipo;
	int Captcha = 0;
	int Runa = 0;
	if (Type >= 100 && Type < 200) {
		Captcha = m->Tipo - 100;
		Type = 5;
	}

	int BattlePassPage = 0;
	if (Type >= 200 && Type < 300) {
		BattlePassPage = m->Tipo - 200;
		Type = 18;
	}

	switch (Type) {
		case 1:
		{			
			UpdateDonate(conn);
		}break;
		case 2:
		{
			SendShopDonate(conn);
		}break;
		case 3:
		{
			SendMailItem(conn);
		}break;
		case 4:
		{
			sendRanking(conn, 0);
		}break;
		case 5:
		{
			VerificarRecaptcha(conn, Captcha);
		}break;
		case 6:
		{
			sendRanking(conn, 1);
		}break;
		case 18: {
			SendBattlePass(conn, BattlePassPage);
		}break;
		case 19: {
			BattlePassReward(conn);
		}break;
		case 29: {
			if (pUser[conn].Mode != USER_PLAY && pUser[conn].Mode >= USER_SELCHAR)
				pUser[conn].Reconnected = TRUE;
		}break;
	}
}