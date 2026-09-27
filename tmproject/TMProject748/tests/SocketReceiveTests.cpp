#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <array>
#include <cstdio>
#include <cstring>
#include "../internal/platform/windows/CPSock.h"

// Only application-owned clocks, window, seed, and diagnostics are substituted.
// Framing, buffering, decoding, and Winsock calls execute production CPSock.
HWND hWndMain = nullptr;
char EncodeByte[4]{};
unsigned int CurrentTime = 0;
unsigned int LastSendTime = 0;
void WYD748_DiagnosticsPacket(const char*, unsigned int, std::size_t) {}
extern unsigned char pKeyWord[512];

int main()
{
    int checks = 0;
    int failures = 0;
    const auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) { ++failures; std::fprintf(stderr, "FAIL socket receive: %s\n", name); }
    };
    {
        CPSock socket;
        std::memcpy(socket.pRecvBuffer, "0123456789abcdefghijkl", 22);
        socket.nProcPosition = 7;
        socket.nRecvPosition = 22;
        socket.RefreshRecvBuffer();
        check(socket.nProcPosition == 0 && socket.nRecvPosition == 15 &&
            std::memcmp(socket.pRecvBuffer, "789abcdefghijkl", 15) == 0,
            "compaction retains the entire unprocessed suffix");
        socket.RefreshRecvBuffer();
        check(socket.nRecvPosition == 15, "repeated compaction is idempotent");
        socket.nProcPosition = 15;
        socket.RefreshRecvBuffer();
        check(socket.nRecvPosition == 0 && socket.nProcPosition == 0,
            "fully consumed receive window becomes empty");
        socket.nProcPosition = 2;
        socket.nRecvPosition = 1;
        socket.RefreshRecvBuffer();
        check(socket.nProcPosition == 2 && socket.nRecvPosition == 1,
            "invalid window is not moved or silently repaired");
    }

    // Encode a 28-byte frame with the established 7.48 byte transform.
    // Deliver it at every split point, retrying each prefix before its tail.
    std::array<unsigned char, 28> plain{};
    plain[0] = 28;
    plain[2] = 0xED;
    plain[4] = 0x91;
    plain[5] = 0x01;
    plain[12] = 42;
    auto encrypted = plain;
    unsigned char sumPlain = 0, sumEncoded = 0;
    for (std::size_t i = 4; i < encrypted.size(); ++i) {
        const auto key = pKeyWord[((pKeyWord[plain[2] * 2] + i - 4) % 256) * 2 + 1];
        const int delta[] = {key * 2, -(key >> 3), key * 4, -(key >> 5)};
        encrypted[i] = static_cast<unsigned char>(plain[i] + delta[i % 4]);
        sumPlain += plain[i];
        sumEncoded += encrypted[i];
    }
    encrypted[3] = static_cast<unsigned char>(sumEncoded - sumPlain);
    plain[3] = encrypted[3];
    for (int split = 0; split < static_cast<int>(encrypted.size()); ++split) {
        CPSock socket;
        socket.Init = 1;
        socket.RecvQueue[0] = 0x12;
        socket.RecvQueue[1] = 0x34;
        std::memcpy(socket.pRecvBuffer, encrypted.data(), split);
        socket.nRecvPosition = split;
        int error = 0, detail = 0;
        for (int retry = 0; retry < 3; ++retry) {
            check(socket.ReadMessage(&error, &detail) == nullptr && error == 0 &&
                socket.RecvCount == 0 && socket.nProcPosition == 0,
                "incomplete frame leaves cursor and rolling key unchanged");
        }
        std::memcpy(socket.pRecvBuffer + split, encrypted.data() + split, encrypted.size() - split);
        socket.nRecvPosition = static_cast<int>(encrypted.size());
        const char* frame = socket.ReadMessage(&error, &detail);
        check(frame && error == 0 && socket.RecvCount == 1 &&
            std::memcmp(frame, plain.data(), plain.size()) == 0,
            "completed frame decodes once with the original key");
        check(socket.ReadMessage(&error, &detail) == nullptr && socket.RecvCount == 1,
            "drained queue does not consume another key");
    }
    {
        CPSock socket;
        socket.Init = 1;
        socket.pRecvBuffer[0] = 0;
        socket.pRecvBuffer[1] = 0;
        socket.nRecvPosition = static_cast<int>(encrypted.size());
        int error = 0, detail = 0;
        check(socket.ReadMessage(&error, &detail) == nullptr && error == 2 &&
            socket.nRecvPosition == 0 && socket.nProcPosition == 0,
            "invalid frame length reports a fatal framing error");
    }
    {
        CPSock socket;
        socket.Init = 1;
        socket.RecvQueue[0] = 0x13;
        std::memcpy(socket.pRecvBuffer, encrypted.data(), encrypted.size());
        socket.nRecvPosition = static_cast<int>(encrypted.size());
        int error = 0, detail = 0;
        check(socket.ReadMessage(&error, &detail) == nullptr && error == 3,
            "invalid rolling keyword reports a protocol error");
    }
    {
        CPSock socket;
        socket.Init = 1;
        auto corrupted = encrypted;
        ++corrupted[3];
        std::memcpy(socket.pRecvBuffer, corrupted.data(), corrupted.size());
        socket.nRecvPosition = static_cast<int>(corrupted.size());
        int error = 0, detail = 0;
        check(socket.ReadMessage(&error, &detail) == nullptr && error == 1 &&
            detail == static_cast<int>(corrupted.size()),
            "invalid checksum rejects the frame before dispatch");
    }
    {
        CPSock socket;
        socket.Init = 1;
        auto corrupted = encrypted;
        ++corrupted[3];
        std::memcpy(socket.pRecvBuffer, corrupted.data(), corrupted.size());
        socket.nRecvPosition = static_cast<int>(corrupted.size());
        int error = 0, detail = 0;
        const PacketView packet = socket.ReadPacketView(&error, &detail);
        check(packet.data == nullptr && packet.size == 0 && error == 1,
            "packet view does not expose a frame with an invalid checksum");
    }

    WSADATA data{};
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
        std::fprintf(stderr, "FAIL socket receive: Winsock initialization\n");
        return 1;
    }
    {
        CPSock socket;
        socket.Sock = static_cast<unsigned int>(::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
        check(socket.Sock != INVALID_SOCKET, "reconnect fixture owns a socket");
        socket.SendQueue[0] = 0x12;
        socket.SendQueue[sizeof(socket.SendQueue) - 1] = 0x56;
        socket.RecvQueue[0] = 0x34;
        socket.RecvQueue[sizeof(socket.RecvQueue) - 1] = 0x78;
        socket.SendCount = 7;
        socket.RecvCount = 9;
        socket.ErrCount = 3;
        socket.Init = 1;
        socket.nSendPosition = 4;
        socket.nRecvPosition = 8;
        socket.CloseSocket();
        check(socket.Sock == 0 && socket.Init == 0 &&
            socket.nSendPosition == 0 && socket.nSentPosition == 0 &&
            socket.nRecvPosition == 0 && socket.nProcPosition == 0 &&
            socket.SendCount == 0 && socket.RecvCount == 0 && socket.ErrCount == 0 &&
            socket.SendQueue[0] == 0 && socket.SendQueue[sizeof(socket.SendQueue) - 1] == 0 &&
            socket.RecvQueue[0] == 0 && socket.RecvQueue[sizeof(socket.RecvQueue) - 1] == 0,
            "socket close removes the previous session's queued keys and cursors");
        check(socket.CloseSocket() == 1, "socket close remains idempotent");
    }
    const SOCKET ownedHandle = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    check(ownedHandle != INVALID_SOCKET, "destructor fixture creates a socket");
    if (ownedHandle != INVALID_SOCKET) {
        {
            CPSock socket;
            socket.Sock = static_cast<unsigned int>(ownedHandle);
        }
        check(closesocket(ownedHandle) == SOCKET_ERROR && WSAGetLastError() == WSAENOTSOCK,
            "socket destructor releases the live handle");
    }
    SOCKET listener = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKET sender = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    int addressSize = sizeof(address);
    bool ready = listener != INVALID_SOCKET && sender != INVALID_SOCKET &&
        bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0 &&
        getsockname(listener, reinterpret_cast<sockaddr*>(&address), &addressSize) == 0 &&
        listen(listener, 1) == 0 &&
        connect(sender, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0;
    SOCKET receiver = ready ? accept(listener, nullptr, nullptr) : INVALID_SOCKET;
    ready = ready && receiver != INVALID_SOCKET;
    check(ready, "loopback socket pair is available");
    const auto acceptPeer = [&] {
        fd_set readable;
        FD_ZERO(&readable);
        FD_SET(listener, &readable);
        timeval wait{2, 0};
        return select(0, &readable, nullptr, nullptr, &wait) == 1
            ? accept(listener, nullptr, nullptr) : INVALID_SOCKET;
    };
    if (ready) {
        CPSock socket;
        socket.Sock = static_cast<unsigned int>(receiver);
        DWORD timeout = 2000;
        setsockopt(receiver, SOL_SOCKET, SO_RCVTIMEO,
            reinterpret_cast<const char*>(&timeout), sizeof(timeout));
        socket.nRecvPosition = RECV_BUFFER_SIZE - 1;
        check(send(sender, "X", 1, 0) == 1 && socket.Receive() == 1 &&
            socket.nRecvPosition == RECV_BUFFER_SIZE &&
            socket.pRecvBuffer[RECV_BUFFER_SIZE - 1] == 'X',
            "read filling the remaining capacity is retained");

        socket.nProcPosition = RECV_BUFFER_SIZE - 1;
        check(send(sender, "Y", 1, 0) == 1 && socket.Receive() == 1 &&
            socket.nProcPosition == 0 && socket.nRecvPosition == 2 &&
            std::memcmp(socket.pRecvBuffer, "XY", 2) == 0,
            "receive reclaims consumed bytes and appends after the suffix");
        u_long nonblocking = 1;
        check(ioctlsocket(receiver, FIONBIO, &nonblocking) == 0,
            "test socket enters asynchronous mode");
        check(socket.Receive() == 1 && socket.nRecvPosition == 2,
            "would-block preserves the connection and buffered bytes");
        using Event = CPSock::EventResult;
        check(socket.HandleNetworkEvent(sender, WSAMAKESELECTREPLY(FD_CLOSE, WSAECONNRESET)) ==
            Event::Ignored && socket.Sock == receiver && socket.nRecvPosition == 2,
            "notification for another socket cannot disconnect the active socket");
        check(socket.HandleNetworkEvent(receiver, WSAMAKESELECTREPLY(FD_READ, 0)) ==
            Event::ReadReady && socket.Sock == receiver,
            "stale readable notification preserves the live socket");
        check(socket.HandleNetworkEvent(receiver, WSAMAKESELECTREPLY(FD_WRITE, 0)) ==
            Event::Ignored && socket.Sock == receiver,
            "initial writable notification is not a disconnect");

        // Exercise outbound rejection and append using the same real TCP pair.
        // The peer consumes complete encrypted frames without running the game.
        check(setsockopt(sender, SOL_SOCKET, SO_RCVTIMEO,
            reinterpret_cast<const char*>(&timeout), sizeof(timeout)) == 0,
            "outbound peer reads have a bounded timeout");
        socket.SendQueue[0] = 0x12;
        socket.SendQueue[1] = 0x34;
        auto request = plain;
        CurrentTime = 0x12345678;
        LastSendTime = 7;
        for (int invalidSize : {-1, 0, 11, 0x10000}) {
            check(socket.AddMessage(reinterpret_cast<char*>(request.data()), invalidSize) == 0 &&
                socket.SendCount == 0 && request == plain && LastSendTime == 7,
                "invalid size does not consume a key or modify the caller frame");
        }
        check(socket.AddMessage(nullptr, static_cast<int>(request.size())) == 0 &&
            socket.SendCount == 0 && LastSendTime == 7,
            "null packet does not consume a key");
        socket.Sock = 0;
        check(socket.AddMessage(reinterpret_cast<char*>(request.data()),
            static_cast<int>(request.size())) == 0 && socket.SendCount == 0 && request == plain,
            "closed socket does not consume a key or modify the caller frame");
        socket.Sock = static_cast<unsigned int>(receiver);
        socket.nSendPosition = SEND_BUFFER_SIZE - 1;
        check(socket.AddMessage(reinterpret_cast<char*>(request.data()),
            static_cast<int>(request.size())) == 0 && socket.SendCount == 0 && request == plain &&
            socket.nSendPosition == SEND_BUFFER_SIZE - 1 && LastSendTime == 7,
            "full queue rejection preserves the key and caller frame");

        // Stage the unsent tail of an earlier write at the end of the queue.
        // Only that tail, followed by the new frame, may reach the peer.
        socket.SendCount = 0;
        socket.nSentPosition = socket.nSendPosition - static_cast<int>(encrypted.size());
        std::memcpy(socket.pSendBuffer + socket.nSentPosition, encrypted.data(), encrypted.size());
        request[12] = 43;
        const bool appended = socket.AddMessage(reinterpret_cast<char*>(request.data()),
            static_cast<int>(request.size())) == 1;
        check(appended && socket.SendCount == 1 && request[2] == 0xED &&
            LastSendTime == CurrentTime,
            "append reclaims the sent prefix and commits exactly one key");
        if (appended) {
            std::array<unsigned char, 56> received{};
            const int receivedSize = recv(sender, reinterpret_cast<char*>(received.data()),
                static_cast<int>(received.size()), MSG_WAITALL);
            check(receivedSize == received.size() &&
                std::memcmp(received.data(), encrypted.data(), encrypted.size()) == 0,
                "pending encrypted suffix is sent once without re-encoding");
            if (receivedSize == received.size()) {
                CPSock peer;
                peer.Init = 1;
                std::memcpy(peer.pRecvBuffer, received.data(), received.size());
                peer.nRecvPosition = receivedSize;
                int error = 0, detail = 0;
                const char* first = peer.ReadMessage(&error, &detail);
                check(first && error == 0 && std::memcmp(first, plain.data(), plain.size()) == 0,
                    "pending frame remains decodable before the appended frame");
                const char* second = peer.ReadMessage(&error, &detail);
                check(second && error == 0 && std::memcmp(second, request.data(), request.size()) == 0,
                    "appended frame preserves the established encryption and ordering");
            }
        }
        const auto receiveFrame = [&] {
            std::array<unsigned char, 28> received{};
            const int size = recv(sender, reinterpret_cast<char*>(received.data()),
                static_cast<int>(received.size()), MSG_WAITALL);
            check(size == received.size(), "accepted frame reaches the TCP peer");
            if (size != received.size()) return;
            CPSock peer;
            peer.Init = 1;
            std::memcpy(peer.pRecvBuffer, received.data(), received.size());
            peer.nRecvPosition = size;
            int error = 0, detail = 0;
            const char* decoded = peer.ReadMessage(&error, &detail);
            check(decoded && error == 0 && std::memcmp(decoded, request.data(), request.size()) == 0,
                "accepted frame decodes to the exact caller payload and header");
        };
        if (appended) {
            const bool next = socket.AddMessage(reinterpret_cast<char*>(request.data()),
                static_cast<int>(request.size())) == 1;
            check(next && socket.SendCount == 2 && request[2] == 0xCB,
                "next accepted frame uses the next queued key");
            if (next) receiveFrame();

            const bool explicitKey = socket.AddMessage(reinterpret_cast<char*>(request.data()),
                static_cast<int>(request.size()), 0x7A) == 1;
            check(explicitKey && socket.SendCount == 2 && request[2] == 0x7A,
                "explicit-key send leaves the automatic sequence unchanged");
            if (explicitKey) receiveFrame();

            std::memset(socket.SendQueue, 0, sizeof(socket.SendQueue));
            const bool noQueue = socket.AddMessage(reinterpret_cast<char*>(request.data()),
                static_cast<int>(request.size())) == 1;
            check(noQueue && socket.SendCount == 2,
                "Go server zero-SecretCode mode does not consume rolling keys");
            if (noQueue) receiveFrame();
        }
        // A write-ready notification must flush an existing frame without a
        // second gameplay action, re-encoding, or advancing the rolling key.
        std::memcpy(socket.pSendBuffer, encrypted.data(), encrypted.size());
        socket.nSendPosition = static_cast<int>(encrypted.size());
        const int priorSendCount = socket.SendCount;
        check(socket.HandleNetworkEvent(receiver, WSAMAKESELECTREPLY(FD_WRITE, 0)) ==
            Event::Ignored && socket.nSendPosition == 0 && socket.nSentPosition == 0 &&
            socket.SendCount == priorSendCount,
            "writable notification drains pending ciphertext without consuming a key");
        request = plain;
        receiveFrame();

        // Force real TCP backpressure with bounded writes. No fake Winsock or
        // window/game process is used; notifications below drive production dispatch.
        const int smallBuffer = 4096;
        check(setsockopt(receiver, SOL_SOCKET, SO_SNDBUF,
            reinterpret_cast<const char*>(&smallBuffer), sizeof(smallBuffer)) == 0,
            "send buffer can be bounded for the backpressure regression");
        std::size_t queuedBytes = 0;
        bool blocked = false, flushOk = true;
        const auto byteAt = [](std::size_t offset) {
            return static_cast<char>((offset * 37) ^ (offset >> 8) ^ (offset >> 16));
        };
        for (int attempt = 0; attempt < 64 && !blocked && flushOk; ++attempt) {
            for (int i = 0; i < SEND_BUFFER_SIZE - 1; ++i)
                socket.pSendBuffer[i] = byteAt(queuedBytes + i);
            socket.nSendPosition = SEND_BUFFER_SIZE - 1;
            queuedBytes += socket.nSendPosition;
            flushOk = socket.SendMessageA();
            blocked = socket.nSendPosition > socket.nSentPosition;
        }
        check(flushOk && blocked, "would-block retains the unsent suffix without reporting failure");
        check(ioctlsocket(sender, FIONBIO, &nonblocking) == 0,
            "backpressure peer uses nonblocking reads");
        std::array<char, 65536> drained{};
        std::size_t receivedBytes = 0;
        bool intact = true, resumed = true;
        const ULONGLONG deadline = GetTickCount64() + 5000;
        while (receivedBytes < queuedBytes && GetTickCount64() < deadline && resumed && intact) {
            fd_set reads, writes;
            FD_ZERO(&reads); FD_SET(sender, &reads);
            FD_ZERO(&writes);
            if (socket.nSendPosition > socket.nSentPosition) FD_SET(receiver, &writes);
            timeval timeoutSlice{0, 100000};
            if (select(0, &reads, &writes, nullptr, &timeoutSlice) == SOCKET_ERROR) {
                resumed = false;
                break;
            }
            if (FD_ISSET(sender, &reads)) {
                const int count = recv(sender, drained.data(), static_cast<int>(drained.size()), 0);
                if (count > 0) {
                    for (int i = 0; i < count; ++i)
                        intact = intact && drained[i] == byteAt(receivedBytes + i);
                    receivedBytes += count;
                } else if (count == 0 || WSAGetLastError() != WSAEWOULDBLOCK) {
                    resumed = false;
                }
            }
            if (FD_ISSET(receiver, &writes))
                resumed = socket.HandleNetworkEvent(receiver, WSAMAKESELECTREPLY(FD_WRITE, 0)) ==
                    Event::Ignored && socket.Sock == receiver;
        }
        check(resumed && intact && receivedBytes == queuedBytes &&
            socket.nSendPosition == 0 && socket.nSentPosition == 0,
            "write-ready retries deliver all backpressured bytes exactly once");
        u_long blocking = 0;
        check(ioctlsocket(sender, FIONBIO, &blocking) == 0, "peer returns to bounded blocking reads");

        socket.nSendPosition = 1;
        socket.nSentPosition = 2;
        check(!socket.SendMessageA() && socket.nSendPosition == 1 && socket.nSentPosition == 2,
            "invalid send cursors fail without silently dropping queued bytes");
        socket.nSendPosition = socket.nSentPosition = 0;
        check(socket.SendMessageA(), "empty send queue is already flushed");
        check(shutdown(receiver, SD_SEND) == 0, "local send shutdown is available");
        check(socket.SendOneMessage(reinterpret_cast<char*>(request.data()),
            static_cast<int>(request.size())) == 0,
            "definitive Winsock send failure is not reported as success");
        check(socket.nSendPosition == request.size() && socket.nSentPosition == 0,
            "failed flush preserves the accepted encrypted frame until disconnect");
        closesocket(sender);
        sender = INVALID_SOCKET;
        // Wait for the explicit FIN, not a timing-dependent sleep.
        fd_set readable;
        FD_ZERO(&readable);
        FD_SET(receiver, &readable);
        timeval wait{2, 0};
        check(select(0, &readable, nullptr, nullptr, &wait) == 1 &&
            socket.HandleNetworkEvent(receiver, WSAMAKESELECTREPLY(FD_READ, 0)) ==
                Event::Disconnected && socket.Sock == 0 && socket.nSendPosition == 0,
            "read EOF closes the socket and requests scene disconnect notification");
        check(socket.HandleNetworkEvent(receiver, WSAMAKESELECTREPLY(FD_CLOSE, 0)) ==
            Event::Ignored, "late close notification cannot disconnect the scene twice");
        receiver = INVALID_SOCKET;
    }
    for (const auto notification : {WSAMAKESELECTREPLY(FD_CLOSE, 0),
        WSAMAKESELECTREPLY(FD_READ, WSAECONNRESET), WSAMAKESELECTREPLY(FD_WRITE, WSAENETDOWN),
        WSAMAKESELECTREPLY(FD_WRITE, 0)}) {
        CPSock socket;
        const SOCKET handle = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        check(handle != INVALID_SOCKET, "event error fixture creates a socket");
        if (handle == INVALID_SOCKET) continue;
        socket.Sock = static_cast<unsigned int>(handle);
        // The unconnected socket also exercises a fatal send during FD_WRITE.
        socket.pSendBuffer[0] = 42;
        socket.nSendPosition = 1;
        check(socket.HandleNetworkEvent(handle, notification) == CPSock::EventResult::Disconnected &&
            socket.Sock == 0, "close, packed errors, and failed write retries disconnect");
    }
    if (ready) {
        // No window exists in this console test. Registration must fail instead
        // of pretending an unmonitored socket is a working game connection.
        CPSock socket;
        char loopback[] = "127.0.0.1";
        u_long nonblockingListener = 1;
        check(ioctlsocket(listener, FIONBIO, &nonblockingListener) == 0,
            "registration failure fixture cannot block in accept");
        check(socket.ConnectServer(loopback, ntohs(address.sin_port), 0, WM_USER + 100) == 0 &&
            socket.Sock == 0, "failed event registration is rejected by ConnectServer");
        SOCKET peer = acceptPeer();
        check(peer != INVALID_SOCKET, "ConnectServer registration failure reached the loopback listener");
        if (peer != INVALID_SOCKET) closesocket(peer);
    }
    if (ready) {
        // The handshake is raw, followed by an ordinary encrypted frame on
        // the same TCP stream. The server expects exactly this byte order.
        SOCKET connected = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        const bool connectedOk = connected != INVALID_SOCKET &&
            connect(connected, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0;
        check(connectedOk,
            "handshake fixture connects to loopback");
        SOCKET peer = connectedOk ? acceptPeer() : INVALID_SOCKET;
        check(peer != INVALID_SOCKET, "handshake fixture accepts the connection");
        if (connected != INVALID_SOCKET && peer != INVALID_SOCKET) {
            CPSock socket;
            socket.Sock = static_cast<unsigned int>(connected);
            check(socket.SendHandshake() && socket.Init == 1,
                "new stream accepts one queued InitCode handshake");
            check(!socket.SendHandshake() && socket.Sock == connected,
                "an initialized stream cannot send a second InitCode");
            socket.SendQueue[0] = 0x12;
            socket.SendQueue[1] = 0x34;
            CurrentTime = 0;
            auto request = plain;
            check(socket.AddMessage(reinterpret_cast<char*>(request.data()),
                static_cast<int>(request.size())) == 1,
                "first encrypted frame follows the handshake");
            std::array<unsigned char, 32> received{};
            DWORD timeout = 2000;
            setsockopt(peer, SOL_SOCKET, SO_RCVTIMEO,
                reinterpret_cast<const char*>(&timeout), sizeof(timeout));
            int count = 0;
            while (count < static_cast<int>(received.size())) {
                const int bytes = recv(peer, reinterpret_cast<char*>(received.data() + count),
                    static_cast<int>(received.size()) - count, 0);
                if (bytes <= 0) break;
                count += bytes;
            }
            const unsigned int code = INIT_CODE;
            check(count == received.size(), "handshake and first frame reach the peer together");
            if (count == received.size()) {
                check(std::memcmp(received.data(), &code, sizeof(code)) == 0,
                    "wire stream starts with exactly one InitCode");
                CPSock decoder;
                decoder.Init = 1;
                decoder.RecvQueue[0] = 0x12;
                decoder.RecvQueue[1] = 0x34;
                std::memcpy(decoder.pRecvBuffer, received.data() + sizeof(code), encrypted.size());
                decoder.nRecvPosition = static_cast<int>(encrypted.size());
                int error = 0, detail = 0;
                const char* decoded = decoder.ReadMessage(&error, &detail);
                check(decoded && error == 0 &&
                    std::memcmp(decoded, request.data(), request.size()) == 0,
                    "first frame after InitCode decodes to the queued packet");
            }
            socket.CloseSocket();
        } else if (connected != INVALID_SOCKET) {
            closesocket(connected);
        }
        if (peer != INVALID_SOCKET) closesocket(peer);

        connected = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        const bool failedFixtureConnected = connected != INVALID_SOCKET &&
            connect(connected, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0;
        check(failedFixtureConnected,
            "failed-handshake fixture connects to loopback");
        peer = failedFixtureConnected ? acceptPeer() : INVALID_SOCKET;
        check(peer != INVALID_SOCKET, "failed-handshake fixture accepts the connection");
        if (connected != INVALID_SOCKET && peer != INVALID_SOCKET) {
            CPSock socket;
            socket.Sock = static_cast<unsigned int>(connected);
            check(shutdown(connected, SD_SEND) == 0, "failed-handshake fixture disables sending");
            check(!socket.SendHandshake() && socket.Sock == 0 && socket.Init == 0 &&
                socket.nSendPosition == 0, "definitive handshake send failure closes the stream");
            fd_set readable;
            FD_ZERO(&readable); FD_SET(peer, &readable);
            timeval wait{2, 0};
            char discarded = 0;
            check(select(0, &readable, nullptr, nullptr, &wait) == 1 &&
                recv(peer, &discarded, 1, 0) == 0,
                "failed handshake never publishes a partial login stream");
        } else if (connected != INVALID_SOCKET) {
            closesocket(connected);
        }
        if (peer != INVALID_SOCKET) closesocket(peer);
    }
    if (sender != INVALID_SOCKET) closesocket(sender);
    if (receiver != INVALID_SOCKET) closesocket(receiver);
    if (listener != INVALID_SOCKET) closesocket(listener);
    WSACleanup();
    if (failures == 0) std::printf("SocketReceiveTests: %d checks PASS\n", checks);
    return failures ? 1 : 0;
}
