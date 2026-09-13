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

void MacroNextSala(int conn, int Type, int Sala) {
	//PERGA N
	if (Type == 1) {
		STRUCT_ITEM Perga;
		memset(&Perga, 0, sizeof(STRUCT_ITEM));

		Perga.sIndex = 3173 + Sala;

		int TargetX = pMob[conn].TargetX;
		int TargetY = pMob[conn].TargetY;

		pUser[conn].Ingame.MacroSala = Sala;
		pUser[conn].Ingame.MacroType = 1;

		if (pUser[conn].Ingame.MacroState == 2 || Perga.sIndex == 3173) {
			pUser[conn].Ingame.MacroState = 1;

			if (RemoveItem(conn, Perga.sIndex, 1) == FALSE) {
				SendClientMessage(conn, "Item não encontrado. Macro desativado");
				pUser[conn].Ingame.MacroState = 0;
				return;
			}
		}

		if ((pMob[conn].extra.ClassMaster == CELESTIAL || pMob[conn].extra.ClassMaster == CELESTIALCS || pMob[conn].extra.ClassMaster == ARCH || pMob[conn].extra.ClassMaster == SCELESTIAL) && pMob[conn].MOB.CurrentScore.Level > -1)
		{
			PutItem(conn, &Perga);
			SendClientMessage(conn, "Apenas Personagens Mortais ");
			pUser[conn].Ingame.MacroState = 2;
			return;
		}

		char UserName[128];
		int UserArea = Sala <= 7 ? GetUserInArea(WaterScrollPosition[0][Sala][0] - 8, WaterScrollPosition[0][Sala][1] - 8, WaterScrollPosition[0][Sala][0] + 8, WaterScrollPosition[0][Sala][1] + 8, UserName) : GetUserInArea(WaterScrollPosition[0][Sala][0] - 12, WaterScrollPosition[0][Sala][1] - 12, WaterScrollPosition[0][Sala][0] + 12, WaterScrollPosition[0][Sala][1] + 12, UserName);

		if (UserArea >= 1)
		{
			SendMsgExp(conn, strFmt("O jogador [%s] Já se encontra na sala", UserName), TNColor::Default, false);
			PutItem(conn, &Perga);
			pUser[conn].Ingame.MacroState = 2;
			return;
		}

		WaterClear1[0][Sala] = 30;

		DoTeleport(conn, WaterScrollPosition[0][Sala][0], WaterScrollPosition[0][Sala][1]);
		SendClientSignalParm(conn, ESCENE_FIELD, _MSG_StartTime, WaterClear1[0][Sala] * 2);

		for (int i = 0; i < MAX_PARTY; i++)
		{
			int partyconn = pMob[conn].PartyList[i];

			if ((pMob[partyconn].extra.ClassMaster == CELESTIAL || pMob[partyconn].extra.ClassMaster == CELESTIALCS || pMob[partyconn].extra.ClassMaster == SCELESTIAL) && pMob[partyconn].MOB.CurrentScore.Level >= -1) {
				SendClientMessage(partyconn, "Apenas Personagens Mortais e Arch");
				continue;
			}

			if (partyconn > 0 && partyconn < MAX_USER && partyconn != conn && pUser[partyconn].Mode == USER_PLAY)
			{
				DoTeleport(partyconn, WaterScrollPosition[0][Sala][0], WaterScrollPosition[0][Sala][1]);
				SendClientSignalParm(partyconn, ESCENE_FIELD, _MSG_StartTime, WaterClear1[0][Sala] * 2);
			}
		}

		if (Sala <= 7)
		{
			GenerateMob(Sala + WATER_N_INITIAL, 0, 0);
			GenerateMob(Sala + WATER_N_INITIAL, 0, 0);
			RebuildGenerator();
		}
		else if (Sala == 8)
		{
			int _rand = rand() % 10;

			if (_rand < 4)
				GenerateMob(WATER_N_INITIAL + 8, 0, 0);

			else if (_rand < 5)
				GenerateMob(WATER_N_INITIAL + 9, 0, 0);

			else if (_rand < 6)
				GenerateMob(WATER_N_INITIAL + 10, 0, 0);

			else
				GenerateMob(WATER_N_INITIAL + 11, 0, 0);
		}
	}
	//PERGA M
	if (Type == 2) {
		STRUCT_ITEM Perga;
		memset(&Perga, 0, sizeof(STRUCT_ITEM));

		Perga.sIndex = 777 + Sala;

		int TargetX = pMob[conn].TargetX;
		int TargetY = pMob[conn].TargetY;

		pUser[conn].Ingame.MacroSala = Sala;
		pUser[conn].Ingame.MacroType = 2;

		if (pUser[conn].Ingame.MacroState == 2 || Perga.sIndex == 777) {
			pUser[conn].Ingame.MacroState = 1;

			if (RemoveItem(conn, Perga.sIndex, 1) == FALSE) {
				SendClientMessage(conn, "Item não encontrado. Macro desativado");
				pUser[conn].Ingame.MacroState = 0;
				return;
			}
		}

		if ((pMob[conn].extra.ClassMaster == CELESTIAL || pMob[conn].extra.ClassMaster == CELESTIALCS || pMob[conn].extra.ClassMaster == SCELESTIAL) && pMob[conn].MOB.CurrentScore.Level > -1)
		{
			PutItem(conn, &Perga);
			SendClientMessage(conn, "Apenas Personagens Mortais e Arch");
			pUser[conn].Ingame.MacroState = 2;
			return;
		}

		char UserName[128];
		int UserArea = Sala <= 7 ? GetUserInArea(WaterScrollPosition[1][Sala][0] - 8, WaterScrollPosition[1][Sala][1] - 8, WaterScrollPosition[1][Sala][0] + 8, WaterScrollPosition[1][Sala][1] + 8, UserName) : GetUserInArea(WaterScrollPosition[1][Sala][0] - 12, WaterScrollPosition[1][Sala][1] - 12, WaterScrollPosition[1][Sala][0] + 12, WaterScrollPosition[1][Sala][1] + 12, UserName);

		if (UserArea >= 1)
		{
			SendMsgExp(conn, strFmt("O jogador [%s] Já se encontra na sala", UserName), TNColor::Default, false);
			PutItem(conn, &Perga);
			pUser[conn].Ingame.MacroState = 2;
			return;
		}

		WaterClear1[1][Sala] = 30;

		DoTeleport(conn, WaterScrollPosition[1][Sala][0], WaterScrollPosition[1][Sala][1]);
		SendClientSignalParm(conn, ESCENE_FIELD, _MSG_StartTime, WaterClear1[1][Sala] * 2);

		for (int i = 0; i < MAX_PARTY; i++)
		{
			int partyconn = pMob[conn].PartyList[i];

			if ((pMob[partyconn].extra.ClassMaster == CELESTIAL || pMob[partyconn].extra.ClassMaster == CELESTIALCS || pMob[partyconn].extra.ClassMaster == SCELESTIAL) && pMob[partyconn].MOB.CurrentScore.Level >= -1) {
				SendClientMessage(partyconn, "Apenas Personagens Mortais e Arch");
				continue;
			}

			if (partyconn > 0 && partyconn < MAX_USER && partyconn != conn && pUser[partyconn].Mode == USER_PLAY)
			{
				DoTeleport(partyconn, WaterScrollPosition[1][Sala][0], WaterScrollPosition[1][Sala][1]);
				SendClientSignalParm(partyconn, ESCENE_FIELD, _MSG_StartTime, WaterClear1[1][Sala] * 2);
			}
		}

		if (Sala <= 7)
		{
			GenerateMob(Sala + WATER_M_INITIAL, 0, 0);
			GenerateMob(Sala + WATER_M_INITIAL, 0, 0);
			RebuildGenerator();
		}
		else if (Sala == 8)
		{
			int _rand = rand() % 10;

			if (_rand < 4)
				GenerateMob(WATER_M_INITIAL + 8, 0, 0);

			else if (_rand < 5)
				GenerateMob(WATER_M_INITIAL + 9, 0, 0);

			else if (_rand < 6)
				GenerateMob(WATER_M_INITIAL + 10, 0, 0);

			else
				GenerateMob(WATER_M_INITIAL + 11, 0, 0);
		}
	}
	//PERGA A
	if (Type == 3) {
		STRUCT_ITEM Perga;
		memset(&Perga, 0, sizeof(STRUCT_ITEM));

		Perga.sIndex = 3182 + Sala;

		int TargetX = pMob[conn].TargetX;
		int TargetY = pMob[conn].TargetY;

		pUser[conn].Ingame.MacroSala = Sala;
		pUser[conn].Ingame.MacroType = 3;

		if (pUser[conn].Ingame.MacroState == 2 || Perga.sIndex == 3182) {
			pUser[conn].Ingame.MacroState = 1;

			if (RemoveItem(conn, Perga.sIndex, 1) == FALSE) {
				SendClientMessage(conn, "Item não encontrado. Macro desativado");
				pUser[conn].Ingame.MacroState = 0;
				return;
			}
		}

		char UserName[128];
		int UserArea = Sala <= 7 ? GetUserInArea(WaterScrollPosition[2][Sala][0] - 8, WaterScrollPosition[2][Sala][1] - 8, WaterScrollPosition[2][Sala][0] + 8, WaterScrollPosition[2][Sala][1] + 8, UserName) : GetUserInArea(WaterScrollPosition[2][Sala][0] - 12, WaterScrollPosition[2][Sala][1] - 12, WaterScrollPosition[2][Sala][0] + 12, WaterScrollPosition[2][Sala][1] + 12, UserName);

		if (UserArea >= 1)
		{
			SendMsgExp(conn, strFmt("O jogador [%s] Já se encontra na sala", UserName), TNColor::Default, false);
			PutItem(conn, &Perga);
			pUser[conn].Ingame.MacroState = 2;
			return;
		}

		WaterClear1[2][Sala] = 30;

		DoTeleport(conn, WaterScrollPosition[2][Sala][0], WaterScrollPosition[2][Sala][1]);
		SendClientSignalParm(conn, ESCENE_FIELD, _MSG_StartTime, WaterClear1[2][Sala] * 2);

		for (int i = 0; i < MAX_PARTY; i++)
		{
			int partyconn = pMob[conn].PartyList[i];

			if (partyconn > 0 && partyconn < MAX_USER && partyconn != conn && pUser[partyconn].Mode == USER_PLAY)
			{
				DoTeleport(partyconn, WaterScrollPosition[2][Sala][0], WaterScrollPosition[2][Sala][1]);
				SendClientSignalParm(partyconn, ESCENE_FIELD, _MSG_StartTime, WaterClear1[2][Sala] * 2);
			}
		}

		if (Sala <= 7)
		{
			GenerateMob(Sala + WATER_A_INITIAL, 0, 0);
			GenerateMob(Sala + WATER_A_INITIAL, 0, 0);
			RebuildGenerator();
		}
		else if (Sala == 8)
		{
			int _rand = rand() % 10;

			if (_rand < 4)
				GenerateMob(WATER_A_INITIAL + 8, 0, 0);

			else if (_rand < 5)
				GenerateMob(WATER_A_INITIAL + 9, 0, 0);

			else if (_rand < 6)
				GenerateMob(WATER_A_INITIAL + 10, 0, 0);

			else
				GenerateMob(WATER_A_INITIAL + 11, 0, 0);
		}
	}
}