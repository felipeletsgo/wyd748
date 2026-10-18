#pragma once

// A separate process captures second-chance exceptions, including fail-fast
// failures that bypass the game's unhandled-exception filter. It never resumes
// gameplay after an unhandled exception or accesses gameplay objects.
#include <Windows.h>
#include <DbgHelp.h>
#include <Shellapi.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <cstdlib>
#include <string>
#include <vector>
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "psapi.lib")

namespace wyd_crash_monitor
{
    using LogSink = void (*)(const char*);
    using WriteDump = BOOL(WINAPI*)(HANDLE, DWORD, HANDLE, MINIDUMP_TYPE,
        const MINIDUMP_EXCEPTION_INFORMATION*, const MINIDUMP_USER_STREAM_INFORMATION*,
        const MINIDUMP_CALLBACK_INFORMATION*);

    inline void Log(const std::wstring& path, const char* line)
    {
        HANDLE file = CreateFileW(path.c_str(), FILE_APPEND_DATA,
            FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE) return;
        DWORD bytes = 0;
        WriteFile(file, line, static_cast<DWORD>(std::strlen(line)), &bytes, nullptr);
        FlushFileBuffers(file);
        CloseHandle(file);
    }

    inline bool IsOwnParent(DWORD pid)
    {
        DWORD parent = 0;
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) return false;
        PROCESSENTRY32W entry{};
        entry.dwSize = sizeof(entry);
        if (Process32FirstW(snapshot, &entry))
        {
            do
            {
                if (entry.th32ProcessID == GetCurrentProcessId())
                {
                    parent = entry.th32ParentProcessID;
                    break;
                }
            } while (Process32NextW(snapshot, &entry));
        }
        CloseHandle(snapshot);
        if (parent != pid || pid == 0) return false;
        HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
        if (!process) return false;
        wchar_t target[MAX_PATH]{};
        wchar_t self[MAX_PATH]{};
        DWORD count = MAX_PATH;
        const BOOL queried = QueryFullProcessImageNameW(process, 0, target, &count);
        const DWORD length = GetModuleFileNameW(nullptr, self, MAX_PATH);
        CloseHandle(process);
        return queried && length > 0 && length < MAX_PATH && _wcsicmp(self, target) == 0;
    }

    inline bool Capture(HANDLE process, const DEBUG_EVENT& event,
        const std::wstring& directory, const std::wstring& logPath, WriteDump writer)
    {
        EXCEPTION_RECORD record = event.u.Exception.ExceptionRecord;
        record.ExceptionRecord = nullptr;
        CONTEXT context{};
        context.ContextFlags = CONTEXT_ALL;
        HANDLE thread = OpenThread(THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE, event.dwThreadId);
        const BOOL gotContext = thread && GetThreadContext(thread, &context);
        const DWORD contextError = gotContext ? ERROR_SUCCESS : GetLastError();
        if (thread) CloseHandle(thread);

        MEMORY_BASIC_INFORMATION memory{};
        const bool image = VirtualQueryEx(process, record.ExceptionAddress, &memory, sizeof(memory)) &&
            memory.State == MEM_COMMIT && memory.Type == MEM_IMAGE && memory.AllocationBase;
        const auto base = image ? reinterpret_cast<ULONG_PTR>(memory.AllocationBase) : 0;
        const auto address = reinterpret_cast<ULONG_PTR>(record.ExceptionAddress);
        char module[MAX_PATH]{};
        if (image)
            GetModuleFileNameExA(process, static_cast<HMODULE>(memory.AllocationBase), module, MAX_PATH);
        char line[1024]{};
        sprintf_s(line,
            "CRASH code=0x%08lX address=%p module=%s base=%p rva=0x%llX fault_tid=%lu capture_tid=%lu capture_pid=%lu first_chance=%lu context_error=%lu\r\n",
            record.ExceptionCode, record.ExceptionAddress, image ? module : "<unmapped>",
            reinterpret_cast<void*>(base), static_cast<unsigned long long>(image ? address - base : 0),
            event.dwThreadId, GetCurrentThreadId(), GetCurrentProcessId(),
            event.u.Exception.dwFirstChance, contextError);
        Log(logPath, line);
        if (record.ExceptionCode == EXCEPTION_ACCESS_VIOLATION && record.NumberParameters >= 2)
        {
            sprintf_s(line, "CRASH memory_operation=%llu memory_address=%p\r\n",
                static_cast<unsigned long long>(record.ExceptionInformation[0]),
                reinterpret_cast<void*>(record.ExceptionInformation[1]));
            Log(logPath, line);
        }
        if (!gotContext) return false;

        SYSTEMTIME now{};
        GetSystemTime(&now);
        wchar_t filename[160]{};
        swprintf_s(filename, L"\\client-crash-%04u%02u%02u-%02u%02u%02u-%03u-p%lu-t%lu.dmp",
            now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond,
            now.wMilliseconds, event.dwProcessId, event.dwThreadId);
        const std::wstring finalPath = directory + filename;
        const std::wstring partial = finalPath + L".partial";
        HANDLE file = CreateFileW(partial.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE)
        {
            sprintf_s(line, "CRASH dump failed stage=create error=%lu\r\n", GetLastError());
            Log(logPath, line);
            return false;
        }
        EXCEPTION_POINTERS pointers{&record, &context};
        MINIDUMP_EXCEPTION_INFORMATION info{event.dwThreadId, &pointers, FALSE};
        const BOOL written = writer(process, event.dwProcessId, file,
            static_cast<MINIDUMP_TYPE>(MiniDumpNormal | MiniDumpWithDataSegs |
                MiniDumpWithThreadInfo | MiniDumpWithUnloadedModules | MiniDumpWithFullMemoryInfo),
            &info, nullptr, nullptr);
        DWORD error = written ? ERROR_SUCCESS : GetLastError();
        LARGE_INTEGER size{};
        bool valid = written != FALSE;
        if (valid && (!GetFileSizeEx(file, &size) || size.QuadPart < sizeof(MINIDUMP_HEADER)))
        {
            error = ERROR_INVALID_DATA;
            valid = false;
        }
        if (valid && !FlushFileBuffers(file))
        {
            error = GetLastError();
            valid = false;
        }
        CloseHandle(file);
        if (valid && !MoveFileW(partial.c_str(), finalPath.c_str()))
        {
            error = GetLastError();
            valid = false;
        }
        sprintf_s(line, "CRASH dump written=%d error=%lu bytes=%lld\r\n", valid ? 1 : 0, error, size.QuadPart);
        Log(logPath, line);
        return valid;
    }

    inline int Run(DWORD pid, const wchar_t* readyName, const wchar_t* directory)
    {
        if (!IsOwnParent(pid)) return 10;
        const std::wstring logPath = std::wstring(directory) + L"\\crash-monitor-p" + std::to_wstring(pid) + L".log";
        HANDLE ready = OpenEventW(EVENT_MODIFY_STATE, FALSE, readyName);
        if (!ready) return 11;
        HMODULE library = LoadLibraryExW(L"dbghelp.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        const auto writer = library ? reinterpret_cast<WriteDump>(GetProcAddress(library, "MiniDumpWriteDump")) : nullptr;
        HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | SYNCHRONIZE, FALSE, pid);
        if (!writer || !process || !DebugActiveProcess(pid))
        {
            char line[128]{};
            sprintf_s(line, "CRASH monitor unavailable stage=attach error=%lu\r\n", GetLastError());
            Log(logPath, line);
            if (process) CloseHandle(process);
            if (library) FreeLibrary(library);
            CloseHandle(ready);
            return 12;
        }
        // A monitor failure must not kill an otherwise healthy game.
        if (!DebugSetProcessKillOnExit(FALSE))
        {
            DebugActiveProcessStop(pid);
            CloseHandle(process);
            FreeLibrary(library);
            CloseHandle(ready);
            return 13;
        }
        bool initialBreakpoint = true;
        bool captured = false;
        for (;;)
        {
            DEBUG_EVENT event{};
            if (!WaitForDebugEvent(&event, 1000))
            {
                if (GetLastError() == ERROR_SEM_TIMEOUT && WaitForSingleObject(process, 0) == WAIT_TIMEOUT)
                    continue;
                break;
            }
            DWORD status = DBG_CONTINUE;
            bool signalReady = false;
            if (event.dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT && event.u.CreateProcessInfo.hFile)
                CloseHandle(event.u.CreateProcessInfo.hFile);
            if (event.dwDebugEventCode == LOAD_DLL_DEBUG_EVENT && event.u.LoadDll.hFile)
                CloseHandle(event.u.LoadDll.hFile);
            if (event.dwDebugEventCode == EXCEPTION_DEBUG_EVENT)
            {
                const auto& exception = event.u.Exception;
                if (initialBreakpoint && exception.dwFirstChance && exception.ExceptionRecord.ExceptionCode == EXCEPTION_BREAKPOINT)
                {
                    initialBreakpoint = false;
                    signalReady = true;
                }
                else
                {
                    status = DBG_EXCEPTION_NOT_HANDLED;
                    if (!exception.dwFirstChance && !captured)
                    {
                        captured = true;
                        Capture(process, event, directory, logPath, writer);
                    }
                }
            }
            const bool exited = event.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT;
            if (!ContinueDebugEvent(event.dwProcessId, event.dwThreadId, status)) break;
            if (signalReady)
            {
                Log(logPath, "CRASH monitor ready mode=external second_chance=1\r\n");
                SetEvent(ready);
            }
            if (exited) break;
        }
        DebugActiveProcessStop(pid);
        CloseHandle(process);
        CloseHandle(ready);
        FreeLibrary(library);
        return 0;
    }

    // Return -1 for ordinary game/test invocations. The helper exits before
    // Direct3D, asset loading, networking or any scene construction.
    inline int RunFromCommandLine()
    {
        int count = 0;
        wchar_t** args = CommandLineToArgvW(GetCommandLineW(), &count);
        if (!args) return -1;
        int result = -1;
        if (count >= 2 && std::wcscmp(args[1], L"--wyd-crash-monitor") == 0)
        {
            result = 14;
            if (count == 5)
            {
                wchar_t* end = nullptr;
                const unsigned long pid = std::wcstoul(args[2], &end, 10);
                if (end && *end == L'\0') result = Run(pid, args[3], args[4]);
            }
        }
        LocalFree(args);
        return result;
    }

    inline bool Start(const char* directory, LogSink log)
    {
        if (IsDebuggerPresent())
        {
            if (log) log("CRASH monitor skipped reason=debugger_present\r\n");
            return false;
        }
        wchar_t executable[MAX_PATH]{};
        const DWORD length = GetModuleFileNameW(nullptr, executable, MAX_PATH);
        if (!length || length >= MAX_PATH || !directory) return false;
        const int count = MultiByteToWideChar(CP_ACP, 0, directory, -1, nullptr, 0);
        if (count <= 0) return false;
        std::vector<wchar_t> output(static_cast<size_t>(count));
        if (!MultiByteToWideChar(CP_ACP, 0, directory, -1, output.data(), count)) return false;
        wchar_t readyName[128]{};
        swprintf_s(readyName, L"Local\\WYD748CrashReady-p%lu-%llu", GetCurrentProcessId(), GetTickCount64());
        HANDLE ready = CreateEventW(nullptr, TRUE, FALSE, readyName);
        if (!ready) return false;
        std::wstring command = L"\"" + std::wstring(executable) + L"\" --wyd-crash-monitor " +
            std::to_wstring(GetCurrentProcessId()) + L" \"" + readyName + L"\" \"" + output.data() + L"\"";
        STARTUPINFOW startup{};
        startup.cb = sizeof(startup);
        PROCESS_INFORMATION helper{};
        const BOOL launched = CreateProcessW(executable, command.data(), nullptr, nullptr, FALSE,
            CREATE_NO_WINDOW, nullptr, nullptr, &startup, &helper);
        bool started = false;
        DWORD error = launched ? ERROR_SUCCESS : GetLastError();
        if (launched)
        {
            HANDLE waits[]{ready, helper.hProcess};
            const DWORD waited = WaitForMultipleObjects(2, waits, FALSE, 5000);
            started = waited == WAIT_OBJECT_0;
            if (!started) error = waited == WAIT_TIMEOUT ? ERROR_TIMEOUT : ERROR_PROCESS_ABORTED;
            CloseHandle(helper.hThread);
            CloseHandle(helper.hProcess);
        }
        CloseHandle(ready);
        if (log)
        {
            char line[160]{};
            sprintf_s(line, "CRASH monitor started=%d pid=%lu error=%lu\r\n", started ? 1 : 0, helper.dwProcessId, error);
            log(line);
        }
        return started;
    }
}
