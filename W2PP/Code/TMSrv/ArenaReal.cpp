#pragma once

#include "ProcessClientMessage.h"
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

ARENAREAL ArenaReal;

int ARTeleport[10][2] = { {148, 4076}, {188, 4076}, {168, 4059},{168, 4029}, {151,4011}, {186,4012}, {189,4028}, {189,4060}, {147,4061}, {146,4027} };
int GrupoPos[4][2] = { {148, 4077}, {187, 4076}, {148, 4012}, {187, 4013} };

void ArenaRealTimer() {

	struct tm when;
	time_t now;
	time(&now);
	when = *localtime(&now);

	// Coloca atÃ¯Â¿Â½ 8 horÃ¯Â¿Â½rios para a arena real. HorÃ¯Â¿Â½rios nÃ¯Â¿Â½o utilizados deixar -1
	ArenaReal.Hora[0] = 6;
	ArenaReal.Hora[1] = 12;
	ArenaReal.Hora[2] = 18;
	ArenaReal.Hora[3] = 19;
	ArenaReal.Hora[4] = 14;
	ArenaReal.Hora[5] = -1;
	ArenaReal.Hora[6] = -1;
	ArenaReal.Hora[7] = -1;

	ArenaReal.Minuto[0] = 0;
	ArenaReal.Minuto[1] = 0;
	ArenaReal.Minuto[2] = 0;
	ArenaReal.Minuto[3] = 0;
	ArenaReal.Minuto[4] = 0;
	ArenaReal.Minuto[5] = 0;
	ArenaReal.Minuto[6] = 0;
	ArenaReal.Minuto[7] = 0;

	ArenaReal.Time[0] = 600;

	ArenaReal.ItemConsolo.sIndex = 4026; //Item dado pra quem perdeu
	ArenaReal.ItemPremio.sIndex = 4029; //Item dado pra quem ganhou
	ArenaReal.HonraConsolo = 10; //Honra dado pra quem perdeu
	ArenaReal.HonraPremio = 50; //Honra dado pra quem ganhou
	ArenaReal.ItemRegistro.sIndex = 0; //Item pra pedir no registro
	ArenaReal.GoldRegistro = 0; //Gold pra pedir no registro
	ArenaReal.EvoMin = 0; //EvoluÃ¯Â¿Â½Ã¯Â¿Â½o minima pra entrar. 0 - mortal, 1 - arch, 2 - cele
	ArenaReal.LevelMin = 351; //level mÃ¯Â¿Â½nimo pra entrar

	if (ArenaReal.State == 0 || ArenaReal.State == 1) {
		for (int i = 0; i < MAX_ARENAS; i++) {
			if (ArenaReal.State == 0 && ArenaReal.Hora[i] == when.tm_hour) {
				if (when.tm_min >= ArenaReal.Minuto[i] && when.tm_min <= (ArenaReal.Minuto[i] + 2)) {
					memset(&ArenaReal.Jogadores, 0, sizeof(JOGADORES));
					SendNotice("Registro da Arena Real Aberto por 3 Minutos!");
					ArenaReal.State = 1;
				}
			}
			if (ArenaReal.State == 1 && ArenaReal.Hora[i] == when.tm_hour) {
				if (when.tm_min >= (ArenaReal.Minuto[i] + 3)) {
					ArenaReal.State = 2;
					SendNotice("Arena Real Iniciada!");
					ArenaReal.Time[1] = ArenaReal.Time[0];
					StartArenaReal();
				}
			}
		}
	}
	if (ArenaReal.State == 2) {
		if (ArenaReal.Time[1] >= 1) {
			ArenaReal.Time[1]--;
		}
		if (ArenaReal.Time[1] <= 0) {
			ArenaRealFinaliza();
		}
	}
}

void StartArenaReal() {

	int Count[4] = { 0,0,0,0 };

	// Verifica se os participantes cadastrados estÃ¯Â¿Â½o online. Se nÃ¯Â¿Â½o estiverem, sÃ¯Â¿Â½o retirados da party
	// Conta quantos participantes estÃ¯Â¿Â½o online em cada equipe
	for (int k = 0; k < LIMITE_PARTIC; k++) {

		if (ArenaReal.Jogadores.Player[0][k] != 0) {
			if (pUser[ArenaReal.Jogadores.Player[0][k]].Mode == USER_PLAY) {
				RemoveParty(ArenaReal.Jogadores.Player[0][k]);
				Count[0]++;
			}
			else {
				ArenaReal.Jogadores.Player[0][k] = 0;
			}
		}

		if (ArenaReal.Jogadores.Player[1][k] != 0) {
			if (pUser[ArenaReal.Jogadores.Player[1][k]].Mode == USER_PLAY) {
				RemoveParty(ArenaReal.Jogadores.Player[1][k]);
				Count[1]++;
			}
			else {
				ArenaReal.Jogadores.Player[1][k] = 0;
			}
		}

		if (ArenaReal.Jogadores.Player[2][k] != 0) {
			if (pUser[ArenaReal.Jogadores.Player[2][k]].Mode == USER_PLAY) {
				RemoveParty(ArenaReal.Jogadores.Player[2][k]);
				Count[2]++;
			}
			else {
				ArenaReal.Jogadores.Player[2][k] = 0;
			}
		}

		if (ArenaReal.Jogadores.Player[3][k] != 0) {
			if (pUser[ArenaReal.Jogadores.Player[3][k]].Mode == USER_PLAY) {
				RemoveParty(ArenaReal.Jogadores.Player[3][k]);
				Count[3]++;
			}
			else {
				ArenaReal.Jogadores.Player[3][k] = 0;
			}
		}
	}

	//Se nÃ¯Â¿Â½o houver pelo menos 1 participante online em cada equipe, nÃ¯Â¿Â½o inicia
	if (Count[0] < 1 || Count[1] < 1 || Count[2] < 1 || Count[3] < 1) {
		SendNotice("NÃ¯Â¿Â½o hÃ¯Â¿Â½ participantes suficiente para ocorrer a Arena Real");
		return;
	}

	int Leaders[4] = {};

	//Monta as Party
	for (int z = 0; z < 4; z++) {
		for (int x = 0; x < LIMITE_PARTIC; x++) {
			int conn = ArenaReal.Jogadores.Player[z][x];

			if (conn == 0)
				continue;

			if (Leaders[z] == 0) {

				char Chave[4096] = { 0, };
				memset(Chave, 0x0, 4096);
				int valorGerado = rand() % (9999 - 1000 + 3333) + 1110;
				std::string s = std::to_string(valorGerado);

				Leaders[z] = conn;

				sprintf_s(Chave, 4096, "%s", s.c_str());
				memset(&pUser[conn].Ingame.PartyPassword, 0, sizeof(pUser[conn].Ingame.PartyPassword));
				strncpy(pUser[conn].Ingame.PartyPassword, Chave, 6);
				pUser[conn].Ingame.GrupoAceitarSolicitacao = TRUE;

				if (pUser[conn].Keys[39] == 0) {
					for (int i = 1; i < 200; i++) {
						if (Parties[i].Index == 0) {
							Parties[i].Index = conn;
							STRCPY_SAFE(Parties[i].Nick, pMob[conn].MOB.MobName, 16);
							Parties[i].Chave = valorGerado;
							pUser[conn].Keys[39] = i;
							pUser[conn].Keys[44] = Parties[i].Chave;
							break;
						}
					}
				}
			}
			else {
				int idx = Leaders[z];

				MSG_SendReqParty sm_cap;
				memset(&sm_cap, 0, sizeof(MSG_SendReqParty));

				sm_cap.ID = ESCENE_FIELD;
				sm_cap.Type = _MSG_SendReqParty;
				sm_cap.Size = sizeof(MSG_SendReqParty);
				sm_cap.Target = idx;
				sm_cap.unk = 1;

				sm_cap.Level = pMob[idx].MOB.CurrentScore.Level;
				sm_cap.MaxHp = pMob[idx].MOB.CurrentScore.MaxHp > 32000 ? ((pMob[idx].MOB.CurrentScore.MaxHp + 1) / 100) : pMob[idx].MOB.CurrentScore.MaxHp;
				sm_cap.Hp = pMob[idx].MOB.CurrentScore.Hp > 32000 ? ((pMob[idx].MOB.CurrentScore.Hp + 1) / 100) : pMob[idx].MOB.CurrentScore.Hp;

				sm_cap.PartyID = idx;
				sm_cap.Target = (short)52428;

				STRCPY_SAFE(sm_cap.MobName, pMob[idx].MOB.MobName, sizeof(sm_cap.MobName));

				if (!pUser[conn].cSock.SendOneMessage((char*)&sm_cap, sizeof(MSG_SendReqParty))) {
					CloseUser(conn);
					ArenaReal.Jogadores.Player[z][x] = 0;
				}
			}
		}
	}

	//Teleporta as Party
	for (int p = 0; p < 4; p++) {
		for (int f = 0; f < LIMITE_PARTIC; f++) {
			int conn = ArenaReal.Jogadores.Player[p][f];

			if (conn == 0)
				continue;

			int fRand = rand() % 3;

			if (pUser[conn].Keys[31] == 0)
				strncpy(pMob[conn].Tab, "ROXO", 36);
			if (pUser[conn].Keys[31] == 1)
				strncpy(pMob[conn].Tab, "LARANJA", 36);
			if (pUser[conn].Keys[31] == 2)
				strncpy(pMob[conn].Tab, "VERDE", 36);
			if (pUser[conn].Keys[31] == 3)
				strncpy(pMob[conn].Tab, "AZUL", 36);

			MSG_CreateMob sm_tb;
			memset(&sm_tb, 0, sizeof(MSG_CreateMob));
			GetCreateMob(conn, &sm_tb);

			GridMulticast(pMob[conn].TargetX, pMob[conn].TargetY, (MSG_STANDARD*)&sm_tb, 0);

			DoTeleport(conn, GrupoPos[p][0] + fRand, GrupoPos[p][1] + fRand);
			pMob[conn].GuildDisable = 1;
			SendClientMessage(conn, "A Arena Real ComeÃ¯Â¿Â½ou!");
			SendClientSignalParm(conn, ESCENE_FIELD, _MSG_StartTime, ArenaReal.Time[1]);
		}
	}
	UpdateArenaRealUI();
}

void ArenaRealFinaliza() {

	int MaxKills = 0;
	int Venceu[4] = {};
	int NumVencedores = 0;
	int Winner = 0;

	for (int i = 0; i < 4; i++) {
		if (ArenaReal.Jogadores.Kills[i] >= MaxKills) {
			MaxKills = ArenaReal.Jogadores.Kills[i];
		}
	}
	for (int x = 0; x < 4; x++) {
		if (ArenaReal.Jogadores.Kills[x] == MaxKills) {
			Venceu[x] = 1;
			Winner = x;
			NumVencedores++;
		}
	}
	if (NumVencedores > 1) {
		Venceu[0] = 0;
		Venceu[1] = 0;
		Venceu[2] = 0;
		Venceu[3] = 0;
		SendNotice("A Arena Real terminou em Empate!");
	}
	if (NumVencedores == 1) {
		if (Winner == 0)
			SendNotice("Arena Real Finalizada! Vencedor: Equipe Roxa");
		if (Winner == 1)
			SendNotice("Arena Real Finalizada! Vencedor: Equipe Laranja");
		if (Winner == 3)
			SendNotice("Arena Real Finalizada! Vencedor: Equipe Verde");
		if (Winner == 4)
			SendNotice("Arena Real Finalizada! Vencedor: Equipe Azul");
	}
	for (int c = 0; c < LIMITE_PARTIC; c++) {
		for (int k = 0; k < 4; k++) {
			int conn = ArenaReal.Jogadores.Player[k][c];

			if (Venceu[k] == 0 && conn != 0) {

				if (pUser[conn].Mode != USER_PLAY)
					continue;

				if (pUser[conn].Keys[30] != 1)
					continue;

				if (ArenaReal.ItemConsolo.sIndex != 0)
					PutItem(conn, &ArenaReal.ItemConsolo);

				if (ArenaReal.HonraConsolo != 0)
					pUser[conn].Honra += ArenaReal.HonraConsolo;

				SendClientMessage(conn, "Obrigado por participar da Arena Real, Mais sorte na prÃ¯Â¿Â½xima!");
				pUser[conn].Keys[30] = 0;
				pUser[conn].Keys[31] = 0;

				DoRecall(conn);
				pMob[conn].MOB.CurrentScore.Hp = 2;
				SendScore(conn);
				SendSetHpMp(conn);
				MSG_CreateMob sm_pp;
				GetCreateMob(conn, &sm_pp);
				GridMulticast(pMob[conn].TargetX, pMob[conn].TargetY, (MSG_STANDARD*)&sm_pp, 0);
			}
			if (Venceu[k] == 1 && conn != 0) {

				if (pUser[conn].Mode != USER_PLAY)
					continue;

				if (pUser[conn].Keys[30] != 1)
					continue;

				if (ArenaReal.ItemPremio.sIndex != 0)
					PutItem(conn, &ArenaReal.ItemPremio);

				if (ArenaReal.HonraPremio != 0)
					pUser[conn].Honra += ArenaReal.HonraPremio;

				SendClientMessage(conn, "ParabÃ¯Â¿Â½ns por vencer na Arena Real!");
				pUser[conn].Keys[30] = 0;
				pUser[conn].Keys[31] = 0;

				DoRecall(conn);
				pMob[conn].MOB.CurrentScore.Hp = 2;
				SendScore(conn);
				SendSetHpMp(conn);
				MSG_CreateMob sm_pp;
				GetCreateMob(conn, &sm_pp);
				GridMulticast(pMob[conn].TargetX, pMob[conn].TargetY, (MSG_STANDARD*)&sm_pp, 0);
			}
		}
	}
	ArenaReal.State = 0;
	memset(&ArenaReal.Jogadores, 0, sizeof(JOGADORES));
}

void KillArenaReal(int killer, int killed) {

	if (killer <= 0 || killer >= MAX_USER)
		return;

	if (pUser[killer].Mode != USER_PLAY)
		return;

	if (pUser[killer].cSock.Sock == 0)
		return;


	if (killed <= 0 || killed >= MAX_USER)
		return;

	if (pUser[killed].Mode != USER_PLAY)
		return;

	if (pUser[killed].cSock.Sock == 0)
		return;

	// 50 = Registro Arena Real
	// 51 = Equipe Arena Real
	// 52 = Pos Equipe Arena Real

	if (pUser[killer].Keys[30] == 0) {
		DoRecall(killer);
		return;
	}

	if (pUser[killed].Keys[30] == 0) {
		DoRecall(killed);
		return;
	}

	if (pUser[killer].Keys[31] == pUser[killed].Keys[31])
		return;

	int Equipe = pUser[killer].Keys[31];

	ArenaReal.Jogadores.Kills[Equipe]++;

	SendNotice(strFmt("Jogador [%s] matou [%s], Equipe [%d] Pontos [%d]", pMob[killer].MOB.MobName, pMob[killed].MOB.MobName, Equipe, ArenaReal.Jogadores.Kills[Equipe]));

	int sRand = rand() % 9;

	pMob[killed].MOB.CurrentScore.Hp = pMob[killed].MOB.CurrentScore.MaxHp;
	pMob[killed].MOB.CurrentScore.Mp = pMob[killed].MOB.CurrentScore.MaxMp;

	SendScore(killed);
	SendSetHpMp(killed);
	DoTeleport(killed, ARTeleport[sRand][0], ARTeleport[sRand][1]);
	MSG_CreateMob sm_pp;
	GetCreateMob(killed, &sm_pp);
	GridMulticast(pMob[killed].TargetX, pMob[killed].TargetY, (MSG_STANDARD*)&sm_pp, 0);

	UpdateArenaRealUI();
}

void UpdateArenaRealUI() {

	//ENVIA ATUALIZAÃ¯Â¿Â½Ã¯Â¿Â½O DA JANELA
	MSG_SendArenaReal m;
	memset(&m, 0, sizeof(MSG_SendArenaReal));

	m.ID = ESCENE_FIELD;
	m.Type = _MSG_SendArenaReal;
	m.Size = sizeof(MSG_SendArenaReal);

	for (int x = 0; x < 4; x++) {
		m.Kills[x] = ArenaReal.Jogadores.Kills[x];
	}

	for (int i = 0; i < 4; i++) {
		for (int z = 0; z < LIMITE_PARTIC; z++) {
			int conn = ArenaReal.Jogadores.Player[i][z];

			if (conn == 0)
				continue;

			if (pUser[conn].Mode != USER_PLAY)
				continue;

			if (!pUser[conn].cSock.SendOneMessage((char*)&m, sizeof(MSG_SendArenaReal))) {
				CloseUser(conn);
				ArenaReal.Jogadores.Player[i][z] = 0;
			}
		}
	}

}

void RegistroArenaReal(int conn) {

	if (conn <= 0 || conn >= MAX_USER)
		return;

	if (pUser[conn].Mode != USER_PLAY)
		return;

	if (pUser[conn].cSock.Sock == 0)
		return;

	if (ArenaReal.State != 1) {
		SendClientMessage(conn, "O registro da Arena Real ainda nÃ¯Â¿Â½o foi liberado!");
		return;
	}

	int Evo = 0;

	if (pMob[conn].extra.ClassMaster == ARCH)
		Evo = 1;

	if (pMob[conn].extra.ClassMaster >= CELESTIAL)
		Evo = 2;

	if (Evo < ArenaReal.EvoMin) {
		SendClientMessage(conn, "Sua evoluÃ¯Â¿Â½Ã¯Â¿Â½o nÃ¯Â¿Â½o Ã¯Â¿Â½ permitida");
		return;
	}

	if (pMob[conn].MOB.CurrentScore.Level < (ArenaReal.LevelMin - 1)) {
		SendClientMessage(conn, strFmt("Level mÃ¯Â¿Â½nimo: %d", ArenaReal.LevelMin));
		return;
	}

	if (pUser[conn].Keys[30] == 1) {
		SendClientMessage(conn, "VocÃ¯Â¿Â½ jÃ¯Â¿Â½ estÃ¯Â¿Â½ registrado!");
		return;
	}

	int LimiteIP = false;

	for (int i = 1; i < MAX_USER; i++) {
		if (pUser[i].Mode < USER_LOGIN)
			continue;

		if (pUser[conn].Mode != USER_PLAY)
			continue;

		if (pUser[conn].IP == pUser[i].IP && pUser[i].Keys[30] == TRUE)
			LimiteIP = true;
	}

	if (LimiteIP) {
		SendClientMessage(conn, "Limite de [1] IP para a Arena Real.");
		return;
	}

	int Team = ArenaReal.Jogadores.Equipe;
	int Registrado = -1;

	for (int x = 0; x < LIMITE_PARTIC; x++) {
		if (ArenaReal.Jogadores.Player[Team][x] == 0) {

			if (ArenaReal.GoldRegistro > 0) {
				if (pMob[conn].MOB.Coin < ArenaReal.GoldRegistro) {
					SendClientMessage(conn, strFmt("VocÃ¯Â¿Â½ precisa de [%d] Gold pra se registrar!", ArenaReal.GoldRegistro));
					return;
				}
			}
			if (ArenaReal.ItemRegistro.sIndex != 0) {
				if (RemoveItem(conn, ArenaReal.ItemRegistro.sIndex, 1) == false) {
					SendClientMessage(conn, strFmt("VocÃ¯Â¿Â½ precisa de um [%s] pra se registrar!", g_pItemList[ArenaReal.ItemRegistro.sIndex].Name));
					return;
				}
			}

			ArenaReal.Jogadores.Player[Team][x] = conn;
			pUser[conn].Keys[30] = 1;
			pUser[conn].Keys[31] = Team;

			Registrado = ArenaReal.Jogadores.Equipe;

			ArenaReal.Jogadores.Equipe++;
			if (ArenaReal.Jogadores.Equipe == 4)
				ArenaReal.Jogadores.Equipe = 0;

			break;
		}
	}
	if (Registrado == 0) {
		SendClientMessage(conn, "VocÃ¯Â¿Â½ foi registrado na equipe [Roxa]");
	}
	if (Registrado == 1) {
		SendClientMessage(conn, "VocÃ¯Â¿Â½ foi registrado na equipe [Laranja]");
	}
	if (Registrado == 2) {
		SendClientMessage(conn, "VocÃ¯Â¿Â½ foi registrado na equipe [Verde]");
	}
	if (Registrado == 3) {
		SendClientMessage(conn, "VocÃ¯Â¿Â½ foi registrado na equipe [Azul]");
	}
	if (Registrado == -1) {
		SendClientMessage(conn, "NÃ¯Â¿Â½o havia vagas para registro");
		return;
	}
	SendNotice(strFmt("[%s] Registrado na Arena Real!", pMob[conn].MOB.MobName));
}