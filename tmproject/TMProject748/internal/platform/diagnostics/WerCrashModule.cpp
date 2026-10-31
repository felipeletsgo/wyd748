#include "WerCrashCapture.h"
#include <DbgHelp.h>
#pragma comment(lib, "dbghelp.lib")

namespace {
void Capture(void* remoteContext, const WER_RUNTIME_EXCEPTION_INFORMATION& info)
{
    wyd_wer::Context context{};
    SIZE_T copied = 0;
    if (!ReadProcessMemory(info.hProcess, remoteContext, &context, sizeof(context), &copied) ||
        copied != sizeof(context) || context.magic != wyd_wer::ContextMagic ||
        context.directory[MAX_PATH - 1] != L'\0' || !context.directory[0]) return;
    const DWORD pid = GetProcessId(info.hProcess), tid = GetThreadId(info.hThread);
    wchar_t path[MAX_PATH]{}, partial[MAX_PATH]{}, logPath[MAX_PATH]{};
    if (_snwprintf_s(path, _TRUNCATE, L"%s\\client-fastfail-p%lu-t%lu.dmp", context.directory, pid, tid) < 0 ||
        _snwprintf_s(partial, _TRUNCATE, L"%s.partial", path) < 0 ||
        _snwprintf_s(logPath, _TRUNCATE, L"%s\\wer-crash.log", context.directory) < 0) return;
    HANDLE file = CreateFileW(partial, GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    DWORD error = file == INVALID_HANDLE_VALUE ? GetLastError() : ERROR_SUCCESS;
    bool written = false;
    if (file != INVALID_HANDLE_VALUE) {
        EXCEPTION_RECORD record = info.exceptionRecord;
        CONTEXT registers = info.context;
        EXCEPTION_POINTERS pointers{&record, &registers};
        MINIDUMP_EXCEPTION_INFORMATION exception{tid, &pointers, FALSE};
        written = MiniDumpWriteDump(info.hProcess, pid, file,
            static_cast<MINIDUMP_TYPE>(MiniDumpNormal | MiniDumpWithDataSegs | MiniDumpWithIndirectlyReferencedMemory),
            &exception, nullptr, nullptr) != FALSE;
        if (!written) error = GetLastError();
        if (written && !FlushFileBuffers(file)) { written = false; error = GetLastError(); }
        CloseHandle(file);
        if (written && !MoveFileExW(partial, path, MOVEFILE_WRITE_THROUGH)) { written = false; error = GetLastError(); }
        if (!written) DeleteFileW(partial);
    }
    HANDLE log = CreateFileW(logPath, FILE_APPEND_DATA, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (log != INVALID_HANDLE_VALUE) {
        char line[256]{};
        const int length = sprintf_s(line, "CRASH code=0x%08lX fault_tid=%lu capture_pid=%lu dump written=%d error=%lu\r\n",
            info.exceptionRecord.ExceptionCode, tid, GetCurrentProcessId(), written ? 1 : 0, error);
        DWORD bytes = 0;
        WriteFile(log, line, static_cast<DWORD>(length), &bytes, nullptr);
        FlushFileBuffers(log);
        CloseHandle(log);
    }
}
}

extern "C" HRESULT CALLBACK OutOfProcessExceptionEventCallback(PVOID context,
    const PWER_RUNTIME_EXCEPTION_INFORMATION info, PBOOL claimed, PWSTR, PDWORD, PDWORD)
{
    if (!claimed || !info) return E_INVALIDARG;
    *claimed = FALSE; // Preserve Windows reporting and its existing consent policy.
    if (info->exceptionRecord.ExceptionCode == 0xC0000409 ||
        info->exceptionRecord.ExceptionCode == 0xC0000374 ||
        info->exceptionRecord.ExceptionCode == 0xC0000602) Capture(context, *info);
    return S_OK;
}
extern "C" HRESULT CALLBACK OutOfProcessExceptionEventSignatureCallback(PVOID,
    const PWER_RUNTIME_EXCEPTION_INFORMATION, DWORD, PWSTR, PDWORD, PWSTR, PDWORD)
{ return E_NOTIMPL; }
extern "C" HRESULT CALLBACK OutOfProcessExceptionEventDebuggerLaunchCallback(PVOID,
    const PWER_RUNTIME_EXCEPTION_INFORMATION, PBOOL custom, PWSTR, PDWORD, PBOOL automatic)
{
    if (custom) *custom = FALSE;
    if (automatic) *automatic = FALSE;
    return S_OK;
}
BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID) { return TRUE; }
