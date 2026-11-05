#pragma once
#include <windows.h>
#include <wininet.h>
#include <cstring>
#include <string>

// MODERNIZACAO_COMPATIVEL: retain the native feed, ZIP extractor and UI.
// Only the pinned executable's WinINet imports use this transport.
namespace native_launcher_https {
constexpr char Host[] = "wyd.forjago.com";
constexpr char HttpBase[] = "http://wyd.forjago.com/patch/";
constexpr char HttpsBase[] = "https://wyd.forjago.com/patch/";
struct Api {
    decltype(&InternetOpenUrlA) openUrl = InternetOpenUrlA;
    decltype(&InternetConnectA) connect = InternetConnectA;
    decltype(&HttpOpenRequestA) openRequest = HttpOpenRequestA;
    decltype(&HttpSendRequestA) send = HttpSendRequestA;
    decltype(&HttpQueryInfoA) query = HttpQueryInfoA;
    decltype(&InternetCloseHandle) close = InternetCloseHandle;
};
inline Api api;
inline bool installed = false;

inline DWORD SecureFlags(DWORD flags)
{
    constexpr DWORD unsafe = INTERNET_FLAG_IGNORE_CERT_CN_INVALID |
        INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP |
        INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS | INTERNET_FLAG_CACHE_IF_NET_FAIL;
    return (flags & ~unsafe) | INTERNET_FLAG_SECURE | INTERNET_FLAG_NO_AUTO_REDIRECT |
        INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_NO_UI;
}

inline bool Success(HINTERNET request)
{
    DWORD status = 0, size = sizeof(status);
    if (api.query(request, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &status, &size, nullptr) && status == 200)
        return true;
    SetLastError(ERROR_HTTP_INVALID_SERVER_RESPONSE);
    return false;
}

inline HINTERNET WINAPI OpenUrl(HINTERNET session, LPCSTR url, LPCSTR headers,
    DWORD length, DWORD flags, DWORD_PTR context)
{
    std::string secure;
    if (url && std::strncmp(url, HttpBase, sizeof(HttpBase) - 1) == 0)
        secure = std::string(HttpsBase) + (url + sizeof(HttpBase) - 1);
    else if (url && std::strncmp(url, HttpsBase, sizeof(HttpsBase) - 1) == 0)
        secure = url;
    else {
        SetLastError(ERROR_INTERNET_INVALID_URL);
        return nullptr;
    }
    HINTERNET result = api.openUrl(session, secure.c_str(), headers, length, SecureFlags(flags), context);
    if (result && !Success(result)) {
        api.close(result);
        SetLastError(ERROR_HTTP_INVALID_SERVER_RESPONSE);
        return nullptr;
    }
    return result;
}

inline HINTERNET WINAPI Connect(HINTERNET session, LPCSTR host, INTERNET_PORT port,
    LPCSTR user, LPCSTR password, DWORD service, DWORD flags, DWORD_PTR context)
{
    if (!host || _stricmp(host, Host) || service != INTERNET_SERVICE_HTTP || (user && *user) || (password && *password) ||
        (port != INTERNET_DEFAULT_HTTP_PORT && port != INTERNET_DEFAULT_HTTPS_PORT && port != 0)) {
        SetLastError(ERROR_INTERNET_INVALID_OPERATION);
        return nullptr;
    }
    return api.connect(session, Host, INTERNET_DEFAULT_HTTPS_PORT, nullptr, nullptr, service, flags, context);
}

inline HINTERNET WINAPI OpenRequest(HINTERNET connection, LPCSTR verb, LPCSTR path,
    LPCSTR version, LPCSTR referrer, LPCSTR* accept, DWORD flags, DWORD_PTR context)
{
    if (!path || (verb && std::strcmp(verb, "GET"))) {
        SetLastError(ERROR_INTERNET_INVALID_URL);
        return nullptr;
    }
    // The native URL parser removes the slash after the host. Preserve that
    // ABI, then root the object path before forwarding it to WinINet.
    const std::string rooted = path[0] == '/' ? path : std::string("/") + path;
    if (rooted.compare(0, 7, "/patch/")) {
        SetLastError(ERROR_INTERNET_INVALID_URL);
        return nullptr;
    }
    return api.openRequest(connection, verb, rooted.c_str(), version, referrer, accept, SecureFlags(flags), context);
}

inline BOOL WINAPI Send(HINTERNET request, LPCSTR headers, DWORD size, LPVOID body, DWORD bodySize)
{
    if (!api.send(request, headers, size, body, bodySize))
        return FALSE;
    return Success(request) ? TRUE : FALSE;
}

inline BOOL WINAPI RejectFtp(HINTERNET, LPCSTR, LPCSTR, BOOL, DWORD, DWORD, DWORD_PTR)
{
    SetLastError(ERROR_INTERNET_INVALID_OPERATION);
    return FALSE;
}

inline bool Install()
{
    if (installed)
        return true;
    auto* image = reinterpret_cast<BYTE*>(GetModuleHandleW(nullptr));
    const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(image);
    if (dos->e_magic != IMAGE_DOS_SIGNATURE)
        return false;
    const auto* pe = reinterpret_cast<const IMAGE_NT_HEADERS32*>(image + dos->e_lfanew);
    if (pe->Signature != IMAGE_NT_SIGNATURE || pe->FileHeader.Machine != IMAGE_FILE_MACHINE_I386 ||
        pe->OptionalHeader.ImageBase != 0x400000 || pe->OptionalHeader.SizeOfImage < 0x30000)
        return false;
    struct Hook { DWORD rva; FARPROC original; FARPROC replacement; };
    const Hook hooks[] = {
        {0x2e4f0, reinterpret_cast<FARPROC>(FtpGetFileA), reinterpret_cast<FARPROC>(RejectFtp)},
        {0x2e500, reinterpret_cast<FARPROC>(HttpSendRequestA), reinterpret_cast<FARPROC>(Send)},
        {0x2e504, reinterpret_cast<FARPROC>(HttpOpenRequestA), reinterpret_cast<FARPROC>(OpenRequest)},
        {0x2e508, reinterpret_cast<FARPROC>(InternetOpenUrlA), reinterpret_cast<FARPROC>(OpenUrl)},
        {0x2e514, reinterpret_cast<FARPROC>(InternetConnectA), reinterpret_cast<FARPROC>(Connect)},
    };
    // Validate all slots before mutation. The pinned PE builder checks their names.
    for (const auto& hook : hooks)
        if (*reinterpret_cast<FARPROC*>(image + hook.rva) != hook.original)
            return false;
    DWORD previous = 0;
    if (!VirtualProtect(image + 0x2e4f0, 0x28, PAGE_READWRITE, &previous))
        return false;
    for (const auto& hook : hooks)
        *reinterpret_cast<FARPROC*>(image + hook.rva) = hook.replacement;
    DWORD ignored = 0;
    if (!VirtualProtect(image + 0x2e4f0, 0x28, previous, &ignored))
        return false; // Bootstrap exits before the native patcher on any failure.
    installed = true;
    return true;
}
} // namespace native_launcher_https
