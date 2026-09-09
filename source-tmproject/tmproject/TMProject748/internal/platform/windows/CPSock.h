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
	unsigned int StartListen(HWND hWnd, int ip, int port, int WSA);
	unsigned int ConnectServer(char* HostAddr, int Port, int ip, int WSA);
	unsigned int SingleConnect(char* HostAddr, int Port, int ip, int WSA);
	int Receive();
	char* ReadMessage(int* ErrorCode, int* ErrorType);
	// Non-owning view over the next framed message; storage remains owned by the
	// receive ring and is invalidated by subsequent buffer operations.
	PacketView ReadPacketView(int* ErrorCode, int* ErrorType);
	int CloseSocket();
	int AddMessage(char* pMsg, int Size);
	int AddMessage(char* pMsg, int Size, int FixedKeyWord);
	bool SendMessageA();
	int SendOneMessage(char* Msg, int Size);
	// Fachada sincrona sem ownership; exige buffer gravavel para o cabecalho.
	// Retorna zero para tamanho invalido, senao preserva o retorno legado.
	int SendPacket(const MutablePacketView& packet);
	int SendOneMessageKeyword(char* Msg, int Size, int Keyword);
	int AddMessage2(char* pMsg, int Size);
	char* ReadMessage2(int* ErrorCode, int* ErrorType);
	void RefreshRecvBuffer();
	void RefreshSendBuffer();

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
