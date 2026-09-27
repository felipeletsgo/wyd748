#include "pch.h"
#include "CPSock.h"
#include "../network/SendBuffer.h"
#include "../network/ReceiveBuffer.h"
#include "../diagnostics/ClientDiagnostics.h"
#include "PacketSendBoundary.h"
#include <climits>
#include <WinSock2.h>
#include "Basedef.h"
#include "TMGlobal.h"
#include "TMLog.h"

int ConnectPort = 0;

unsigned char pKeyWord[512] = { // 7.xx keys
    0x84, 0x87, 0x37, 0xd7, 0xea, 0x79, 0x91, 0x7d, 0x4b, 0x4b, 0x85, 0x7d, 0x87, 0x81, 0x91, 0x7c, 0x0f, 0x73, 0x91, 0x91, 0x87, 0x7d, 0x0d, 0x7d, 0x86, 0x8f, 0x73, 0x0f, 0xe1, 0xdd, 0x85, 0x7d,
	0x05, 0x7d, 0x85, 0x83, 0x87, 0x9c, 0x85, 0x33, 0x0d, 0xe2, 0x87, 0x19, 0x0f, 0x79, 0x85, 0x86, 0x37, 0x7d, 0xd7, 0xdd, 0xe9, 0x7d, 0xd7, 0x7d, 0x85, 0x79, 0x05, 0x7d, 0x0f, 0xe1, 0x87, 0x7e,
	0x23, 0x87, 0xf5, 0x79, 0x5f, 0xe3, 0x4b, 0x83, 0xa3, 0xa2, 0xae, 0x0e, 0x14, 0x7d, 0xde, 0x7e, 0x85, 0x7a, 0x85, 0xaf, 0xcd, 0x7d, 0x87, 0xa5, 0x87, 0x7d, 0xe1, 0x7d, 0x88, 0x7d, 0x15, 0x91,
	0x23, 0x7d, 0x87, 0x7c, 0x0d, 0x7a, 0x85, 0x87, 0x17, 0x7c, 0x85, 0x7d, 0xac, 0x80, 0xbb, 0x79, 0x84, 0x9b, 0x5b, 0xa5, 0xd7, 0x8f, 0x05, 0x0f, 0x85, 0x7e, 0x85, 0x80, 0x85, 0x98, 0xf5, 0x9d,
	0xa3, 0x1a, 0x0d, 0x19, 0x87, 0x7c, 0x85, 0x7d, 0x84, 0x7d, 0x85, 0x7e, 0xe7, 0x97, 0x0d, 0x0f, 0x85, 0x7b, 0xea, 0x7d, 0xad, 0x80, 0xad, 0x7d, 0xb7, 0xaf, 0x0d, 0x7d, 0xe9, 0x3d, 0x85, 0x7d,
	0x87, 0xb7, 0x23, 0x7d, 0xe7, 0xb7, 0xa3, 0x0c, 0x87, 0x7e, 0x85, 0xa5, 0x7d, 0x76, 0x35, 0xb9, 0x0d, 0x6f, 0x23, 0x7d, 0x87, 0x9b, 0x85, 0x0c, 0xe1, 0xa1, 0x0d, 0x7f, 0x87, 0x7d, 0x84, 0x7a,
	0x84, 0x7b, 0xe1, 0x86, 0xe8, 0x6f, 0xd1, 0x79, 0x85, 0x19, 0x53, 0x95, 0xc3, 0x47, 0x19, 0x7d, 0xe7, 0x0c, 0x37, 0x7c, 0x23, 0x7d, 0x85, 0x7d, 0x4b, 0x79, 0x21, 0xa5, 0x87, 0x7d, 0x19, 0x7d,
	0x0d, 0x7d, 0x15, 0x91, 0x23, 0x7d, 0x87, 0x7c, 0x85, 0x7a, 0x85, 0xaf, 0xcd, 0x7d, 0x87, 0x7d, 0xe9, 0x3d, 0x85, 0x7d, 0x15, 0x79, 0x85, 0x7d, 0xc1, 0x7b, 0xea, 0x7d, 0xb7, 0x7d, 0x85, 0x7d,
	0x85, 0x7d, 0x0d, 0x7d, 0xe9, 0x73, 0x85, 0x79, 0x05, 0x7d, 0xd7, 0x7d, 0x85, 0xe1, 0xb9, 0xe1, 0x0f, 0x65, 0x85, 0x86, 0x2d, 0x7d, 0xd7, 0xdd, 0xa3, 0x8e, 0xe6, 0x7d, 0xde, 0x7e, 0xae, 0x0e,
	0x0f, 0xe1, 0x89, 0x7e, 0x23, 0x7d, 0xf5, 0x79, 0x23, 0xe1, 0x4b, 0x83, 0x0c, 0x0f, 0x85, 0x7b, 0x85, 0x7e, 0x8f, 0x80, 0x85, 0x98, 0xf5, 0x7a, 0x85, 0x1a, 0x0d, 0xe1, 0x0f, 0x7c, 0x89, 0x0c,
	0x85, 0x0b, 0x23, 0x69, 0x87, 0x7b, 0x23, 0x0c, 0x1f, 0xb7, 0x21, 0x7a, 0x88, 0x7e, 0x8f, 0xa5, 0x7d, 0x80, 0xb7, 0xb9, 0x18, 0xbf, 0x4b, 0x19, 0x85, 0xa5, 0x91, 0x80, 0x87, 0x81, 0x87, 0x7c,
	0x0f, 0x73, 0x91, 0x91, 0x84, 0x87, 0x37, 0xd7, 0x86, 0x79, 0xe1, 0xdd, 0x85, 0x7a, 0x73, 0x9b, 0x05, 0x7d, 0x0d, 0x83, 0x87, 0x9c, 0x85, 0x33, 0x87, 0x7d, 0x85, 0x0f, 0x87, 0x7d, 0x0d, 0x7d,
	0xf6, 0x7e, 0x87, 0x7d, 0x88, 0x19, 0x89, 0xf5, 0xd1, 0xdd, 0x85, 0x7d, 0x8b, 0xc3, 0xea, 0x7a, 0xd7, 0xb0, 0x0d, 0x7d, 0x87, 0xa5, 0x87, 0x7c, 0x73, 0x7e, 0x7d, 0x86, 0x87, 0x23, 0x85, 0x10,
	0xd7, 0xdf, 0xed, 0xa5, 0xe1, 0x7a, 0x85, 0x23, 0xea, 0x7e, 0x85, 0x98, 0xad, 0x79, 0x86, 0x7d, 0x85, 0x7d, 0xd7, 0x7d, 0xe1, 0x7a, 0xf5, 0x7d, 0x85, 0xb0, 0x2b, 0x37, 0xe1, 0x7a, 0x87, 0x79,
	0x84, 0x7d, 0x73, 0x73, 0x87, 0x7d, 0x23, 0x7d, 0xe9, 0x7d, 0x85, 0x7e, 0x02, 0x7d, 0xdd, 0x2d, 0x87, 0x79, 0xe7, 0x79, 0xad, 0x7c, 0x23, 0xda, 0x87, 0x0d, 0x0d, 0x7b, 0xe7, 0x79, 0x9b, 0x7d,
	0xd7, 0x8f, 0x05, 0x7d, 0x0d, 0x34, 0x8f, 0x7d, 0xad, 0x87, 0xe9, 0x7c, 0x85, 0x80, 0x85, 0x79, 0x8a, 0xc3, 0xe7, 0xa5, 0xe8, 0x6b, 0x0d, 0x74, 0x10, 0x73, 0x33, 0x17, 0x0d, 0x37, 0x21, 0x19
};

namespace
{
	bool WYD748_HasRollingEncodeSeed()
	{
		// Ghidra FUN_00424c2c/FUN_00424642 test the four EncodeByte bytes as one
		// 32-bit value.  Testing `if (EncodeByte)` tests the array address instead,
		// which is permanently true and desynchronizes encryption after packet 16.
		unsigned int seed = 0;
		memcpy(&seed, EncodeByte, sizeof(seed));
		return seed != 0;
	}

	unsigned char WYD748_ComputeRollingQueueByte(const char* queue)
	{
		// Preserve signed-char arithmetic from the 32-bit native 7.48 executable;
		// only the final low byte belongs to the keyword protocol.
		const int value = (queue[15] % 2)
			? queue[11] + queue[13] - queue[9] + 4
			: queue[3] + queue[1] + queue[5] - 87;
		return static_cast<unsigned char>(value);
	}

	unsigned char WYD748_ComputeDynamicEncodeByte()
	{
		// This is the exact post-queue derivation used by native 7.48 after the
		// first sixteen directional keywords have been consumed.
		char divisor = EncodeByte[0];
		if (!divisor)
			divisor = EncodeByte[3];
		if (!divisor)
			divisor = 13;

		int value = EncodeByte[2] + EncodeByte[3] - EncodeByte[1] * divisor;
		if (EncodeByte[2] + EncodeByte[3] == EncodeByte[1] * divisor)
			value = EncodeByte[0];
		return static_cast<unsigned char>(value);
	}
}

CPSock::CPSock()
{
	Sock = NULL;
	Init = 0;
	pSendBuffer = (char*)malloc(SEND_BUFFER_SIZE);
	pRecvBuffer = (char*)malloc(RECV_BUFFER_SIZE);

	memset(pSendBuffer, 0, SEND_BUFFER_SIZE);
	memset(pRecvBuffer, 0, RECV_BUFFER_SIZE);

	nSendPosition = 0;
	nSentPosition = 0;
	nRecvPosition = 0;
	nProcPosition = 0;
	memset(SendQueue, 0, MAX_KEYWORD_QUEUE);
	memset(RecvQueue, 0, MAX_KEYWORD_QUEUE);
	SendCount = 0;
	RecvCount = 0;
	ErrCount = 0;
}

CPSock::~CPSock()
{
	CloseSocket();
	if (pSendBuffer)
	{
		free(pSendBuffer);
		pSendBuffer = NULL;
	}
	if (pRecvBuffer)
	{
		free(pRecvBuffer);
		pRecvBuffer = NULL;
	}
}

bool CPSock::WSAInitialize()
{
	WSAData WSAData;

	return WSAStartup(MAKEWORD(1, 1), &WSAData) == 0;
}

unsigned int CPSock::ConnectServer(char* HostAddr, int Port, int ip, int WSA)
{
	sockaddr_in InAddr;
	memset((char*)& InAddr, 0, sizeof(InAddr));
	sockaddr_in local_sin;
	memset((char*)& local_sin, 0, sizeof(local_sin));

	CloseSocket();

	InAddr.sin_addr.S_un.S_addr = inet_addr(HostAddr);
	InAddr.sin_family = AF_INET;
	InAddr.sin_port = htons(Port);
	SOCKET tSock = socket(2, 1, 0);

	if (tSock == -1)
	{
		MessageBoxA(0, "Initialize socket fai", "ERROR", 0);
		return 0;
	}

	local_sin.sin_family = AF_INET;
	local_sin.sin_addr.S_un.S_addr = ip;
	local_sin.sin_port = 0;

	if (bind(tSock, (const struct sockaddr*)&local_sin, 16) != -1
		|| (ConnectPort += 10,
			local_sin.sin_port = htons(ConnectPort + 5000),
			bind(tSock, (const struct sockaddr*)&local_sin, 16) != -1)
		|| (ConnectPort += 10,
			local_sin.sin_port = htons(ConnectPort + 5000),
			bind(tSock, (const struct sockaddr*)&local_sin, 16) != -1))
	{
		if (tSock == -1)
		{
			return 0;
		}
		else if (connect(tSock, (const struct sockaddr*)&InAddr, 16) >= 0)
		{
			if (WSAAsyncSelect(tSock, hWndMain, WSA, FD_READ | FD_WRITE | FD_CLOSE) == 0)
			{
				Sock = tSock;
				return SendHandshake() ? tSock : 0;
			}
			else
			{
				closesocket(tSock);
				Sock = 0;
				return 0;
			}
		}
		else
		{
			WSAGetLastError();
			closesocket(tSock);
			Sock = 0;
			return 0;
		}
	}
	else
	{
		MessageBoxA(0, "Binding fail", "ERROR", 0);
		closesocket(tSock);
		return 0;
	}

	return 1;
}

bool CPSock::SendHandshake()
{
	// Only a freshly connected stream may begin a handshake. Its unsent
	// bytes share the normal queue so the first encrypted frame follows them.
	if (!Sock || Init || nSendPosition != 0 || nSentPosition != 0)
		return false;
	unsigned int code = INIT_CODE;
	static_assert(sizeof(code) == 4, "The 7.48 handshake must contain four bytes");
	if (!pSendBuffer || !AddMessage2(reinterpret_cast<char*>(&code), sizeof(code)) || !SendMessageA())
	{
		CloseSocket();
		return false;
	}
	Init = 1;
	return true;
}

int CPSock::Receive()
{
	if (!Sock || !pRecvBuffer || !receive_buffer::HasValidWindow(
		nProcPosition, nRecvPosition, RECV_BUFFER_SIZE))
		return 0;

	// Reclaim consumed bytes while preserving an incomplete trailing frame.
	RefreshRecvBuffer();
	if (nRecvPosition >= RECV_BUFFER_SIZE)
		return 0;

	int Rest = RECV_BUFFER_SIZE - nRecvPosition;
	int tReceiveSize = recv(Sock, &pRecvBuffer[nRecvPosition], Rest, 0);
	// Zero means orderly peer shutdown. A stale asynchronous notification may
	// find no bytes available; that is not a disconnect.
	if (tReceiveSize == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK)
		return 1;
	if (tReceiveSize <= 0)
		return 0;

	nRecvPosition += tReceiveSize;
	return 1;
}

char* CPSock::ReadMessage(int* ErrorCode, int* ErrorType)
{
	if (!ErrorCode || !ErrorType || !pRecvBuffer || !receive_buffer::HasValidWindow(
		nProcPosition, nRecvPosition, RECV_BUFFER_SIZE))
		return nullptr;

	*ErrorCode = 0;
	if (nProcPosition >= nRecvPosition)
	{
		nRecvPosition = 0;
		nProcPosition = 0;
		return 0;
	}

	if (!Init)
	{
		if (nRecvPosition - nProcPosition < 4)
			return 0;

		unsigned int InitCode = *(unsigned int*)&pRecvBuffer[nProcPosition];
		if (InitCode != INIT_CODE)
		{
			*ErrorCode = 2;
			*ErrorType = InitCode;
			return 0;
		}

		Init = 1;
		nProcPosition += 4;
	}

	if ((unsigned int)(nRecvPosition - nProcPosition) < sizeof(MSG_STANDARD))
		return 0;

	unsigned short Size = *(unsigned short*)&pRecvBuffer[nProcPosition];
	// Validate and wait for the entire frame before consuming a rolling key.
	// TCP may deliver the same header across several receive notifications.
	if (Size >= RECV_BUFFER_SIZE || Size < sizeof(MSG_STANDARD))
	{
		nRecvPosition = 0;
		nProcPosition = 0;
		*ErrorCode = 2;
		*ErrorType = Size;
		return 0;
	}
	if (Size > nRecvPosition - nProcPosition)
		return 0;

	unsigned char iKeyWord = pRecvBuffer[nProcPosition + 2];
	unsigned char KeyWord = pKeyWord[iKeyWord * 2];
	unsigned char CheckSum = pRecvBuffer[nProcPosition + 3];
	unsigned int SockType = *(unsigned short*)&pRecvBuffer[nProcPosition + 4];
	unsigned int SockID = *(unsigned short*)&pRecvBuffer[nProcPosition + 6];

	if (RecvQueue[0] != 0)
	{
		unsigned char qKeyword;
		if (RecvCount < MAX_KEYWORD_QUEUE)
		{
			qKeyword = static_cast<unsigned char>(RecvQueue[RecvCount++]);
		}
		else if (WYD748_HasRollingEncodeSeed())
		{
			qKeyword = WYD748_ComputeDynamicEncodeByte();
		}
		else
		{
			qKeyword = WYD748_ComputeRollingQueueByte(RecvQueue);
		}

		if (static_cast<unsigned char>(qKeyword ^ 0xFF) != iKeyWord)
		{
			*ErrorCode = 3;
			*ErrorType = Size;
			return 0;
		}
	}

	char* pMsg = &pRecvBuffer[nProcPosition];
	nProcPosition += Size;
	if (nRecvPosition <= nProcPosition)
	{
		nRecvPosition = 0;
		nProcPosition = 0;
	}

	unsigned char Sum1 = 0;
	unsigned char Sum2 = 0;
	int pos = KeyWord;
	for (int i = sizeof(int); i < Size; i++, pos++)
	{
		Sum2 += pMsg[i];

		int rst = pos % 256;
		unsigned char Trans = pKeyWord[rst * 2 + 1];

		int mod = i & 0x3;

		if (mod == 0) 
			pMsg[i] = pMsg[i] - (Trans << 1);
		if (mod == 1) 
			pMsg[i] = pMsg[i] + (Trans >> 3);
		if (mod == 2) 
			pMsg[i] = pMsg[i] - (Trans << 2);
		if (mod == 3) 
			pMsg[i] = pMsg[i] + (Trans >> 5);

		Sum1 += pMsg[i];
	}

	if ((unsigned char)(Sum2 - Sum1) != CheckSum)
	{
		*ErrorCode = 1;
		*ErrorType = Size;
		return nullptr;
	}

	return pMsg;
}

int CPSock::CloseSocket()
{
	nSendPosition = 0;
	nSentPosition = 0;
	nRecvPosition = 0;
	nProcPosition = 0;
	memset(SendQueue, 0, sizeof(SendQueue));
	memset(RecvQueue, 0, sizeof(RecvQueue));
	SendCount = 0;
	RecvCount = 0;
	ErrCount = 0;
	Init = 0;
	if (Sock)
		closesocket(Sock);
	Sock = 0;

	return 1;
}

int CPSock::AddMessage(char* pMsg, int Size)
{
	int Keyword = 0;
	if (SendQueue[0])
	{
		if (SendCount < MAX_KEYWORD_QUEUE)
		{
			Keyword = static_cast<unsigned char>(SendQueue[SendCount]) ^ 0xFF;
		}
		else
		{
			// Native FUN_00424c2c switches to EncodeByte only when its four-byte
			// value is non-zero; otherwise it derives the stable queue fallback.
			const unsigned char rawKeyword = WYD748_HasRollingEncodeSeed()
				? WYD748_ComputeDynamicEncodeByte()
				: WYD748_ComputeRollingQueueByte(SendQueue);
			Keyword = rawKeyword ^ 0xFF;
		}
	}

	const int accepted = AddMessage(pMsg, Size, Keyword);
	// A local rejection emits no frame and must not consume its rolling key.
	// Enqueued bytes keep their key even when the immediate flush is partial.
	if (accepted && SendQueue[0] && SendCount < MAX_KEYWORD_QUEUE)
		++SendCount;
	return accepted;
}

// Borrows writable pMsg, frames/encrypts into the owned queue, and tries a flush.
// Bounds failures are rejected before writing the header or appending a frame.
int CPSock::AddMessage(char* pMsg, int Size, int FixedKeyWord)
{
	if (!Sock)
	{
		ErrCount = 10;

		return 0;
	}

	// Reclaim only the sent prefix before checking capacity. Pending bytes are
	// already encrypted and retain their original order and representation.
	if (pMsg && send_buffer::Compact(pSendBuffer, SEND_BUFFER_SIZE,
		nSendPosition, nSentPosition) && send_buffer::CanAppendPacket(
		Size, nSendPosition, SEND_BUFFER_SIZE, sizeof(MSG_STANDARD)))
	{
		unsigned char iKeyWord = FixedKeyWord;
		if (!FixedKeyWord)
			iKeyWord = rand() % 256;

		unsigned char KeyWord = pKeyWord[iKeyWord * 2];
		
		auto packet = reinterpret_cast<MSG_STANDARD*>(pMsg);
		packet->Size = Size;
		packet->KeyWord = iKeyWord;
		packet->CheckSum = 0;

		packet->Tick = CurrentTime;
		LastSendTime = CurrentTime;

		unsigned char Sum1 = 0;
		unsigned char Sum2 = 0;

		int pos = KeyWord;
		int i = 4;

		while (i < Size)
		{
			Sum1 += pMsg[i];

			int rst = pos % 256;
			unsigned char Trans = pKeyWord[rst * 2 + 1];
			int mod = i & 3;

			if (!mod)
				pSendBuffer[i + nSendPosition] = pMsg[i] + 2 * Trans;
			else if (mod == 1)
				pSendBuffer[i + nSendPosition] = pMsg[i] - ((int)Trans >> 3);
			else if (mod == 2)
				pSendBuffer[i + nSendPosition] = pMsg[i] + 4 * Trans;
			else if (mod == 3)
				pSendBuffer[i + nSendPosition] = pMsg[i] - ((int)Trans >> 5);

			Sum2 += pSendBuffer[i++ + nSendPosition];
			++pos;
		}

		unsigned char checkSum = Sum2 - Sum1;
		packet->CheckSum = checkSum;
		memcpy(&pSendBuffer[nSendPosition], pMsg, 4u);

		nSendPosition += Size;

		SendMessageA();
		return 1;
	}

	ErrCount = 1;
	return 0;
}

bool CPSock::SendMessageA()
{
	if (!Sock)
	{
		nSendPosition = 0;
		nSentPosition = 0;

		return false;
	}

	if (!send_buffer::Compact(pSendBuffer, SEND_BUFFER_SIZE, nSendPosition, nSentPosition))
		return false;

	// Keep sending positive short writes until drained or blocked. Winsock
	// rearms FD_WRITE after WSAEWOULDBLOCK, not after every partial write.
	while (nSentPosition < nSendPosition)
	{
		const int sent = send(Sock, pSendBuffer + nSentPosition, nSendPosition - nSentPosition, 0);
		if (sent == SOCKET_ERROR)
			return WSAGetLastError() == WSAEWOULDBLOCK;
		if (sent == 0)
			return false;
		nSentPosition += sent;
	}
	nSendPosition = 0;
	nSentPosition = 0;
	return true;
}

CPSock::EventResult CPSock::HandleNetworkEvent(WPARAM socket, LPARAM notification)
{
	// Messages already queued for a different socket must not affect this one.
	if (!Sock || socket != Sock)
		return EventResult::Ignored;

	const int event = WSAGETSELECTEVENT(notification);
	if (WSAGETSELECTERROR(notification) == 0 && event != FD_CLOSE)
	{
		if (event == FD_READ)
		{
			if (Receive())
				return EventResult::ReadReady;
		}
		else if (event != FD_WRITE || SendMessageA())
			return EventResult::Ignored;
	}

	CloseSocket();
	return EventResult::Disconnected;
}

int CPSock::SendPacket(const MutablePacketView& packet)
{
    // Validate before AddMessage consumes a key or writes the header.
    // The sender owns writable storage: legacy encoding updates its header
    // without retaining the pointer after this call.
    const int result = SendValidatedPacket(packet, sizeof(MSG_STANDARD),
        [this](char* data, int size) { return SendOneMessage(data, size) != 0; });
    if (result != 0)
        WYD748_DiagnosticsPacket("TX", packet.opcode, packet.size);
    return result;
}

PacketView CPSock::ReadPacketView(int* ErrorCode, int* ErrorType)
{
	char* message = ReadMessage(ErrorCode, ErrorType);
	if (message == nullptr)
		return {};

	const auto* standard = reinterpret_cast<const MSG_STANDARD*>(message);
	PacketView packet{ standard->Type, message, static_cast<std::size_t>(standard->Size) };
	WYD748_DiagnosticsPacket("RX", packet.opcode, packet.size);
	return packet;
}

int CPSock::SendOneMessage(char* Msg, int Size)
{
	// Still flush after rejection, but do not mask a failure to enqueue this
	// message. AddMessage retains its internal flush.
	return send_buffer::EnqueueAndFlush(
		[&] { return AddMessage(Msg, Size); }, [this] { return SendMessageA(); });
}

int CPSock::AddMessage2(char* pMsg, int Size)
{
	if (!Sock || !pMsg || !send_buffer::CanAppendRaw(Size, nSendPosition, SEND_BUFFER_SIZE))
		return 0;

	memcpy(&pSendBuffer[nSendPosition], pMsg, Size);
	nSendPosition += Size;

	return 1;
}

void CPSock::RefreshRecvBuffer()
{
	if (!pRecvBuffer || !receive_buffer::HasValidWindow(
		nProcPosition, nRecvPosition, RECV_BUFFER_SIZE))
		return;

	const int left = nRecvPosition - nProcPosition;
	if (nProcPosition > 0 && left > 0)
		memmove(pRecvBuffer, &pRecvBuffer[nProcPosition], left);
	nProcPosition = 0;
	nRecvPosition = left;
}
