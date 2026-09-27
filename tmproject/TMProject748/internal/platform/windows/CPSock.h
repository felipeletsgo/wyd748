#pragma once

#define RECV_BUFFER_SIZE 131072
#define SEND_BUFFER_SIZE 131072

#define MAX_KEYWORD_QUEUE 16

#define INIT_CODE 521270033

#include "../../application/ports/PacketView.h"

class CPSock
{
public:
	CPSock();
	~CPSock();

	bool WSAInitialize();
	unsigned int ConnectServer(char* HostAddr, int Port, int ip, int WSA);
	// Starts a fresh connected stream; a blocked handshake remains queued.
	bool SendHandshake();
	int Receive();
	enum class EventResult { Ignored, ReadReady, Disconnected };
	EventResult HandleNetworkEvent(WPARAM socket, LPARAM notification);
	char* ReadMessage(int* ErrorCode, int* ErrorType);
	// Non-owning view over the next framed message; storage remains owned by the
	// receive ring and is invalidated by subsequent buffer operations.
	PacketView ReadPacketView(int* ErrorCode, int* ErrorType);
	int CloseSocket();
	int AddMessage(char* pMsg, int Size);
	int AddMessage(char* pMsg, int Size, int FixedKeyWord);
	bool SendMessageA();
	int SendOneMessage(char* Msg, int Size);
	// Synchronous, non-owning facade; requires writable header storage.
	// Invalid sizes return zero; otherwise preserve the send result.
	int SendPacket(const MutablePacketView& packet);
	int AddMessage2(char* pMsg, int Size);
	void RefreshRecvBuffer();

	unsigned int Sock;
	char* pSendBuffer;
	char* pRecvBuffer;
	int nSendPosition;
	int nRecvPosition;
	int nProcPosition;
	int nSentPosition;
	int Init;

	char SendQueue[MAX_KEYWORD_QUEUE];
	char RecvQueue[MAX_KEYWORD_QUEUE];

	int SendCount;
	int RecvCount;
	int ErrCount;
};
