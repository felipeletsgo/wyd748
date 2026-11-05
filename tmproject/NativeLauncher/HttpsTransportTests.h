#pragma once
#include "HttpsTransport.h"

namespace https_test {
inline DWORD status = 200, sentFlags = 0, closed = 0, calls = 0;
inline INTERNET_PORT sentPort = 0;
inline std::string sentUrl, sentPath;
inline bool failTls = false;
inline HINTERNET handle = reinterpret_cast<HINTERNET>(0x1234);
inline HINTERNET WINAPI Url(HINTERNET, LPCSTR url, LPCSTR, DWORD, DWORD flags, DWORD_PTR)
{
    ++calls; sentUrl = url; sentFlags = flags;
    if (failTls) { SetLastError(ERROR_INTERNET_SEC_CERT_CN_INVALID); return nullptr; }
    return handle;
}
inline HINTERNET WINAPI Connect(HINTERNET, LPCSTR, INTERNET_PORT port, LPCSTR, LPCSTR, DWORD, DWORD, DWORD_PTR)
{ ++calls; sentPort = port; return handle; }
inline HINTERNET WINAPI Request(HINTERNET, LPCSTR, LPCSTR path, LPCSTR, LPCSTR, LPCSTR*, DWORD flags, DWORD_PTR)
{ ++calls; sentPath = path; sentFlags = flags; return handle; }
inline BOOL WINAPI Send(HINTERNET, LPCSTR, DWORD, LPVOID, DWORD) { return TRUE; }
inline BOOL WINAPI Query(HINTERNET, DWORD, LPVOID out, LPDWORD, LPDWORD)
{ *static_cast<DWORD*>(out) = status; return TRUE; }
inline BOOL WINAPI Close(HINTERNET) { ++closed; return TRUE; }
}

inline void RunHttpsTests()
{
    namespace net = native_launcher_https;
    using namespace https_test;
    const auto original = net::api;
    net::api = {Url, Connect, Request, Send, Query, Close};
    Check(net::OpenUrl(handle, "http://wyd.forjago.com/patch/update.htm", nullptr, 0, 0, 0) == handle,
        "feed opens through HTTPS adapter");
    Check(sentUrl == "https://wyd.forjago.com/patch/update.htm", "upgrade feed scheme before network access");
    Check((sentFlags & INTERNET_FLAG_SECURE) != 0, "feed enables TLS");
    for (const char* url : {"http://attacker.test/patch/update.htm", "http://wyd.forjago.com.attacker.test/patch/a",
        "ftp://wyd.forjago.com/patch/a", "file:///patch/a", "https://wyd.forjago.com/other/a"}) {
        const auto before = calls;
        Check(net::OpenUrl(handle, url, nullptr, 0, 0, 0) == nullptr && calls == before,
            "reject foreign host or protocol without a request");
    }
    failTls = true;
    Check(!net::OpenUrl(handle, net::HttpBase, nullptr, 0, 0, 0) &&
        GetLastError() == ERROR_INTERNET_SEC_CERT_CN_INVALID, "certificate failure propagates without fallback");
    failTls = false;
    for (DWORD code : {301u, 302u, 307u, 404u, 500u}) {
        status = code;
        const auto before = closed;
        Check(!net::OpenUrl(handle, net::HttpsBase, nullptr, 0, 0, 0) && closed == before + 1,
            "reject redirect and error bodies, release URL handle");
        Check(!net::Send(handle, nullptr, 0, nullptr, 0), "reject redirect and error responses for ZIP requests");
    }
    status = 200;
    Check(net::Connect(handle, net::Host, 80, "", "", INTERNET_SERVICE_HTTP, 0, 0) == handle && sentPort == 443,
        "ZIP and guild connections use port 443");
    Check(!net::Connect(handle, net::Host, 21, nullptr, nullptr, INTERNET_SERVICE_FTP, 0, 0), "disable FTP fallback");
    Check(!net::Connect(handle, "attacker.test", 80, nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 0), "reject foreign connect host");
    Check(!net::RejectFtp(handle, "a", "b", FALSE, 0, 0, 0), "reject direct FTP downloads");
    const DWORD unsafe = INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID |
        INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP | INTERNET_FLAG_CACHE_IF_NET_FAIL;
    for (const char* path : {"/patch/guilds.txt", "/patch/patch-48018.zip"}) {
        Check(net::OpenRequest(handle, "GET", path, nullptr, nullptr, nullptr, unsafe, 0) == handle,
            "native guild and ZIP request paths remain supported");
        Check((sentFlags & unsafe) == 0 && (sentFlags & INTERNET_FLAG_SECURE) &&
            (sentFlags & INTERNET_FLAG_NO_AUTO_REDIRECT), "ZIP keeps certificate validation and rejects redirects");
    }
    Check(net::Send(handle, nullptr, 0, nullptr, 0), "accept HTTP 200");
    // Native 0x401b49 removes the first three slashes, then sscanf at
    // 0x401b89 stores a relative object path passed at 0x401d1f.
    Check(net::OpenRequest(handle, nullptr, "patch/guilds.txt", "HTTP/1.1", nullptr, nullptr, 0x84400100, 0) == handle,
        "native relative guild path is accepted");
    Check(sentPath == "/patch/guilds.txt", "native guild path is rooted before WinINet");
    Check(net::OpenRequest(handle, nullptr, "patch/patch-48101.zip", "HTTP/1.1", nullptr, nullptr, 0x84400100, 0) == handle,
        "native relative ZIP path is accepted");
    Check(sentPath == "/patch/patch-48101.zip", "native ZIP path is rooted before WinINet");
    net::api = original;
    Check(!net::Install(), "reject IAT installation in an unrelated executable");
}
