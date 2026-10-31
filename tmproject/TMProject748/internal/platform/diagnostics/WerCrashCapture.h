#pragma once

#include <Windows.h>
#include <werapi.h>
#include <cstdio>
#include <cwchar>
#include <cstdlib>
#pragma comment(lib, "wer.lib")
#pragma comment(lib, "advapi32.lib")

namespace wyd_wer {
constexpr DWORD ContextMagic = 0x57444331;
constexpr wchar_t ModuleName[] = L"WydCrashReporter.dll";
constexpr wchar_t RegistryPath[] = L"Software\\Microsoft\\Windows\\Windows Error Reporting\\RuntimeExceptionHelperModules";
struct Context {
    DWORD magic = ContextMagic;
    wchar_t directory[MAX_PATH]{};
};
inline Context context;
inline wchar_t modulePath[MAX_PATH]{};
inline bool installed = false;
inline void Uninstall();

inline HRESULT Install(const char* directory)
{
    if (installed) return S_OK;
    if (!directory || !MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, directory,
        -1, context.directory, MAX_PATH)) return E_INVALIDARG;
    const DWORD length = GetModuleFileNameW(nullptr, modulePath, MAX_PATH);
    if (!length || length >= MAX_PATH) return HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
    wchar_t* slash = std::wcsrchr(modulePath, L'\\');
    if (!slash) return HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
    *++slash = L'\0';
    if (wcscat_s(modulePath, ModuleName)) return HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
    if (GetFileAttributesW(modulePath) == INVALID_FILE_ATTRIBUTES)
        return HRESULT_FROM_WIN32(GetLastError());
    // Register only this application's reporter for the current user. Keep
    // Windows crash reporting and consent policy under Windows control.
    HKEY key = nullptr;
    LONG error = RegCreateKeyExW(HKEY_CURRENT_USER, RegistryPath, 0, nullptr, 0,
        KEY_SET_VALUE, nullptr, &key, nullptr);
    if (error != ERROR_SUCCESS) return HRESULT_FROM_WIN32(error);
    const DWORD value = 0;
    error = RegSetValueExW(key, modulePath, 0, REG_DWORD,
        reinterpret_cast<const BYTE*>(&value), sizeof(value));
    RegCloseKey(key);
    if (error != ERROR_SUCCESS) return HRESULT_FROM_WIN32(error);
    const HRESULT result = WerRegisterRuntimeExceptionModule(modulePath, &context);
    installed = SUCCEEDED(result);
    if (installed) std::atexit(Uninstall);
    return result;
}

inline void Uninstall()
{
    if (installed) WerUnregisterRuntimeExceptionModule(modulePath, &context);
    installed = false;
}
}
