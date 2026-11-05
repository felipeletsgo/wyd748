#pragma once

// Classifies one WSAAsyncSelect notification for the active CPSock stream.
// The decision is pure; CPSock performs the Winsock calls.
namespace socket_event
{
    // Values of FD_READ, FD_WRITE, FD_CONNECT and FD_CLOSE in winsock2.h.
    // CPSock.cpp checks them against the system definitions at compile time.
    constexpr int kRead = 0x01;
    constexpr int kWrite = 0x02;
    constexpr int kConnect = 0x10;
    constexpr int kClose = 0x20;

    enum class Step { Ignore, Connected, Read, Write, DrainThenClose, Close };

    // A connecting stream accepts only FD_CONNECT. Winsock posts read, write
    // and close events only after the connection completes, so such events
    // while connecting belong to a closed socket whose handle value was reused.
    // A graceful FD_CLOSE first drains bytes that are still in the socket.
    inline Step Classify(bool active, bool sameSocket, bool connecting, int event, int error)
    {
        if (!active || !sameSocket)
            return Step::Ignore;
        if (connecting && event != kConnect)
            return Step::Ignore;
        if (error != 0)
            return Step::Close;
        switch (event)
        {
        case kConnect: return Step::Connected;
        case kRead: return Step::Read;
        case kWrite: return Step::Write;
        case kClose: return Step::DrainThenClose;
        default: return Step::Ignore;
        }
    }
}
