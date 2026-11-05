#pragma once
#include <windows.h>
#include <wininet.h>

namespace channel_status {
// Blocking transport for status workers and the isolated diagnostic probe.
inline int Fetch(const char* url, char* output, int capacity, DWORD timeoutMs = 1500)
{
    if (!url || !output || capacity <= 0) return 0;
    output[0] = 0;
    const auto session = InternetOpenA("WYD-status", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
    if (!session) return 0;
    if (!InternetSetOptionA(session, INTERNET_OPTION_CONNECT_TIMEOUT, &timeoutMs, sizeof timeoutMs) ||
        !InternetSetOptionA(session, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeoutMs, sizeof timeoutMs) ||
        !InternetSetOptionA(session, INTERNET_OPTION_SEND_TIMEOUT, &timeoutMs, sizeof timeoutMs)) {
        InternetCloseHandle(session);
        return 0;
    }
    const auto request = InternetOpenUrlA(session, url, nullptr, 0,
        INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
    DWORD received = 0;
    const bool success = request && InternetReadFile(request, output,
        static_cast<DWORD>(capacity - 1), &received);
    if (request) InternetCloseHandle(request);
    InternetCloseHandle(session);
    output[success ? received : 0] = 0;
    return success ? 1 : 0;
}
}
