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
#include <string.h>
#include "Server.h"

using namespace std;

void GerarRecaptcha()
{
    //srand(time(NULL));
    random_words = rand() % 3;
}

void VerificarRecaptcha(int conn, int word) {

    if (conn <= 0 || conn >= MAX_USER)
        return;

    if (pUser[conn].Mode != USER_PLAY)
        return;

    if (pUser[conn].cSock.Sock == 0)
        return;

    if (word < 0 || word > 5)
        return;

    if (pUser[conn].CaptchaDelay != 0)
    {
        int isTime = GetTickCount64() - pUser[conn].CaptchaDelay;

        if (isTime < 120000) {
            SendClientMessage(conn, "Wait 2 min for a new attempt.");
            return;
        }
    }

    int wordsPos = word;
    int keywordsPos = pUser[conn].Carptcha;

    char words[6][11] = {
        "AIRPLANE", "SHIP", "MOTORCYCLE", "BIKE", "CAR", "TRAIN"
    };

    bool Correct = false;

    if (!strcmp(words[wordsPos], Keywords[keywordsPos]))
        Correct = true;

    if (Correct == true) {

        pUser[conn].CaptchaDelay = 0;
        if (pUser[conn].Territorio == 1) {

            /*if (pMob[conn].MOB.BaseScore.Level < 350) {
                SendClientMessage(conn, "Only Lv 351+");
                return;
            }*/

            for (int z = 0; z < MAX_USER; z++)
            {
                if (pUser[z].Mode != USER_PLAY)
                    continue;

                if (!pUser[z].cSock.Sock)
                    continue;

                if (pMob[z].LanNFlag == 1)
                {
                    if (pUser[z].IP == pUser[conn].IP) {
                        SendClientMessage(conn, "Limit of [01] Account per IP");
                        return;
                    }
                }
            }

           // SendClientSignalParm(conn, ESCENE_FIELD, _MSG_StartTime, LanTime);
            DoTeleport(conn, (3639 + rand() % 5 - 3), (3639 + rand() % 5 - 3));
            pMob[conn].LanNFlag = 1;
            pUser[conn].Territorio = 0;
            return;
        }
        if (pUser[conn].Territorio == 2) {

            for (int z = 0; z < MAX_USER; z++)
            {
                if (pUser[z].Mode != USER_PLAY)
                    continue;

                if (!pUser[z].cSock.Sock)
                    continue;

                if (pMob[z].LanMFlag == 1)
                {
                    if (pUser[z].IP == pUser[conn].IP) {
                        SendClientMessage(conn, "Limit of [01] Account per IP");
                        return;
                    }
                }
            }

           // SendClientSignalParm(conn, ESCENE_FIELD, _MSG_StartTime, LanTime);
            DoTeleport(conn, (3782 + rand() % 5 - 3), (3527 + rand() % 5 - 3));
            pMob[conn].LanMFlag = 1;
            pUser[conn].Territorio = 0;
            return;
        }
        if (pUser[conn].Territorio == 3) {

            for (int z = 0; z < MAX_USER; z++)
            {
                if (pUser[z].Mode != USER_PLAY)
                    continue;

                if (!pUser[z].cSock.Sock)
                    continue;

                if (pMob[z].LanAFlag == 1)
                {
                    if (pUser[z].IP == pUser[conn].IP) {
                        SendClientMessage(conn, "Limit of [01] Account per IP");
                        return;
                    }
                }
            }

           // SendClientSignalParm(conn, ESCENE_FIELD, _MSG_StartTime, LanTime);
            DoTeleport(conn, (3911 + rand() % 5 - 3), (3655 + rand() % 5 - 3));
            pMob[conn].LanAFlag = 1;
            pUser[conn].Territorio = 0;
            return;
        }
        if (pUser[conn].Territorio == 4) {

            if (EventStatus == 0) {
                SendClientMessage(conn, "Event Off");
                return;
            }
            int Connections = 0;
            for (int z = 0; z < MAX_USER; z++)
            {
                if (pUser[z].Mode != USER_PLAY)
                    continue;

                if (pUser[z].DiariaState == 1)
                {
                    if (pUser[z].IP == pUser[conn].IP) {
                        Connections++;
                    }
                }
            }
            if (Connections >= 2) {
                SendClientMessage(conn, "Limit of [02] Account per IP");
                return;
            }
            SendMsgExp(conn, "Welcome to Hit Event", TNColor::Default, false);
            int _rand = rand() % 2;
            if (_rand == 0) {
                DoTeleport(conn, 1311 + rand() % 2, 1499 + rand() % 2);
            }
            else {
                DoTeleport(conn, 1372 + rand() % 2, 1499 + rand() % 2);
            }

            pUser[conn].DiariaState = 1;
            pUser[conn].Territorio = 0;
            return;
        }
    }
    pUser[conn].CaptchaDelay = GetTickCount64();
    SendClientMessage(conn, "Sorry, incorrect answer");
}

void SendRecaptcha(int conn, int territorio) {

    if (conn <= 0 || conn >= MAX_USER)
        return;

    if (pUser[conn].Mode != USER_PLAY)
        return;

    if (pUser[conn].cSock.Sock == 0)
        return;

    MSG_Recaptcha sm;

    memset(&sm, 0, sizeof(MSG_Recaptcha));

    sm.Type = _MSG_Recaptcha;
    sm.Size = sizeof(MSG_Recaptcha);
    sm.ID = conn;

    // srand(time(NULL));
    int Word = rand() % 30;

    pUser[conn].Carptcha = Word;
    pUser[conn].Territorio = territorio;

    sm.state = random_words;
    sm.word = Word;

    int Size = sm.Size;

    if (Size > sizeof(MSG_Recaptcha))
    {
        sm.Size = 0;
        return;
    }

    if (!pUser[conn].cSock.AddMessage((char*)&sm, sizeof(MSG_Recaptcha)))
        CloseUser(conn);
}