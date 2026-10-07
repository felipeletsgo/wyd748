#pragma once

#include "Structures.h"
#include "../../application/ports/PacketView.h"

TMVector3 ComputeNormalVector(TMVector3 v1, TMVector3 v2, TMVector3 v3);
TMVector3 TMVector3Cross(const TMVector3* pV1, const TMVector3* pV2);

void SendOneMessage(char* Msg, int Size);

// Synchronous send through the legacy limiter. Does not keep the pointer;
// needs a writable buffer because the socket edits the header. An invalid size is not sent.
void SendPacket(const MutablePacketView& packet);

void GetSoundAndPlay(int soundId, int priority, int flag);
void GetSoundAndPlayIfNot(int soundId, int priority, int flag);

// This function must be here because of the TMVector3 argument.
int BASE_IsInView(TMVector3 vec, float fRadius);
