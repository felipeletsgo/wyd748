#pragma once
#include <cstdio>

// Read-only production download probe. Never extracts ZIPs or starts the game.
inline HINTERNET WINAPI ExpiredCertificateProbe(HINTERNET session, LPCSTR, LPCSTR headers,
    DWORD length, DWORD flags, DWORD_PTR context)
{
    return InternetOpenUrlA(session, "https://expired.badssl.com/", headers, length, flags, context);
}

inline void RunLiveHttpsProbe()
{
    namespace net = native_launcher_https;
    HINTERNET session = InternetOpenA("WYD-Native-HTTPS-Probe", INTERNET_OPEN_TYPE_PRECONFIG,
        nullptr, nullptr, 0);
    Check(session != nullptr, "open live WinINet session");
    if (!session) return;
    DWORD timeout = 10000;
    InternetSetOptionA(session, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
    InternetSetOptionA(session, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));
    const char* paths[] = {"/patch/update.htm", "/patch/guilds.txt", "/patch/patch-48018.zip"};
    const wchar_t* outputs[] = {L"feed.bin", L"guilds.bin", L"patch.bin"};
    for (int i = 0; i < 3; ++i) {
        HINTERNET connection = nullptr;
        HINTERNET request = nullptr;
        if (i == 0) {
            request = net::OpenUrl(session, "http://wyd.forjago.com/patch/update.htm", nullptr, 0, 0, 0);
        } else {
            connection = net::Connect(session, net::Host, 80, nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 0);
            if (connection) {
                request = net::OpenRequest(connection, "GET", paths[i], nullptr, nullptr, nullptr, 0, 0);
                if (request && !net::Send(request, nullptr, 0, nullptr, 0)) {
                    InternetCloseHandle(request); request = nullptr;
                }
            }
        }
        std::printf("HTTPS %s: %s (error %lu)\n", paths[i], request ? "connected" : "failed", GetLastError());
        Check(request != nullptr, "real TLS request succeeds through native transport");
        if (request) {
            FILE* file = nullptr;
            Check(_wfopen_s(&file, outputs[i], L"wb") == 0 && file, "create isolated probe output");
            if (file) {
                char buffer[8192]; DWORD count = 0; BOOL ok = TRUE;
                while ((ok = InternetReadFile(request, buffer, sizeof(buffer), &count)) && count)
                    Check(std::fwrite(buffer, 1, count, file) == count, "save probe bytes");
                Check(ok != FALSE, "complete real TLS download");
                std::fclose(file);
            }
            InternetCloseHandle(request);
        }
        if (connection) InternetCloseHandle(connection);
    }
    // Use a public expired-certificate fixture beneath the URL policy. The
    // production adapter must strip caller flags that would bypass validation.
    const auto original = net::api.openUrl;
    net::api.openUrl = ExpiredCertificateProbe;
    const auto invalid = net::OpenUrl(session, net::HttpBase, nullptr, 0,
        INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | INTERNET_FLAG_IGNORE_CERT_CN_INVALID, 0);
    const DWORD error = GetLastError();
    std::printf("Expired certificate fixture: handle=%p error=%lu\n", invalid, error);
    Check(!invalid && (error == ERROR_INTERNET_SEC_CERT_DATE_INVALID ||
        error == ERROR_INTERNET_SEC_INVALID_CERT || error == ERROR_INTERNET_INVALID_CA),
        "real invalid certificate fails despite caller ignore flags");
    if (invalid) InternetCloseHandle(invalid);
    net::api.openUrl = original;
    InternetCloseHandle(session);
}
