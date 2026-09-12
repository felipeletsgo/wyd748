#pragma once

#include "Structures.h"
#include "../../application/ports/PacketView.h"

TMVector3 ComputeNormalVector(TMVector3 v1, TMVector3 v2, TMVector3 v3);
TMVector3 TMVector3Cross(const TMVector3* pV1, const TMVector3* pV2);
float disTanceSq(float stX, float stY, float taX, float taY);

void SendOneMessage(char* Msg, int Size);

// Envio sincrono com o limitador legado. Nao retém o ponteiro; exige buffer
// gravavel pois o socket modifica o cabecalho. Tamanho invalido nao e enviado.
void SendPacket(const MutablePacketView& packet);

void GetSoundAndPlay(int soundId, int priority, int flag);
void GetSoundAndPause(int soundId, int priority, int flag);
void GetSoundAndPlayIfNot(int soundId, int priority, int flag);
void GetSoundAndPauseIfNot(int soundId, int priority, int flag);

// This func need's to be here because of the TMVector3 argument.
int BASE_IsInView(TMVector3 vec, float fRadius);
