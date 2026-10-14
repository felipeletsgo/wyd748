#pragma once

// Process-lifetime crash capture. No gameplay objects, locks or allocations are
// accessed by the exception filter. DbgHelp runs only on the prepared worker.
#include <Windows.h>
#include <DbgHelp.h>
#include <cstdio>
#include <cstring>

namespace wyd_crash
{
    using LogSink = void (*)(const char*);
    using WriteDump = BOOL(WINAPI*)(HANDLE, DWORD, HANDLE, MINIDUMP_TYPE,
        const MINIDUMP_EXCEPTION_INFORMATION*, const MINIDUMP_USER_STREAM_INFORMATION*,
        const MINIDUMP_CALLBACK_INFORMATION*);

    struct CaptureState
    {
        HANDLE request = nullptr;
        HANDLE completed = nullptr;
        HMODULE dbghelp = nullptr;
        WriteDump writeDump = nullptr;
        LogSink log = nullptr;
        char directory[MAX_PATH]{};
        EXCEPTION_RECORD record{};
        CONTEXT context{};
        EXCEPTION_POINTERS pointers{};
        DWORD faultThread = 0;
        DWORD workerThread = 0;
        volatile LONG claimed = 0;
        bool installed = false;
    };

    inline CaptureState state;

    inline void Log(const char* text)
    {
        if (state.log)
            state.log(text);
    }

    inline DWORD WINAPI Worker(void*)
    {
        if (WaitForSingleObject(state.request, INFINITE) != WAIT_OBJECT_0)
            return 1;

        char line[1024]{};
        MEMORY_BASIC_INFORMATION memory{};
        const auto address = reinterpret_cast<ULONG_PTR>(state.record.ExceptionAddress);
        const bool mapped = VirtualQuery(state.record.ExceptionAddress, &memory, sizeof(memory)) != 0;
        const auto base = mapped ? reinterpret_cast<ULONG_PTR>(memory.AllocationBase) : 0;
        char module[MAX_PATH]{};
        if (mapped)
            GetModuleFileNameA(static_cast<HMODULE>(memory.AllocationBase), module, MAX_PATH);
        sprintf_s(line,
            "CRASH code=0x%08lX address=%p module=%s base=%p rva=0x%llX fault_tid=%lu capture_tid=%lu\r\n",
            state.record.ExceptionCode, state.record.ExceptionAddress, module,
            reinterpret_cast<void*>(base), static_cast<unsigned long long>(address - base),
            state.faultThread, GetCurrentThreadId());
        Log(line);
        // Access violations carry the attempted operation and address, not a payload.
        if (state.record.ExceptionCode == EXCEPTION_ACCESS_VIOLATION && state.record.NumberParameters >= 2)
        {
            sprintf_s(line, "CRASH memory_operation=%llu memory_address=%p\r\n",
                static_cast<unsigned long long>(state.record.ExceptionInformation[0]),
                reinterpret_cast<void*>(state.record.ExceptionInformation[1]));
            Log(line);
        }

        SYSTEMTIME now{};
        GetSystemTime(&now);
        char partial[MAX_PATH]{};
        char finalPath[MAX_PATH]{};
        const int pathResult = _snprintf_s(finalPath, sizeof(finalPath), _TRUNCATE,
            "%s\\client-crash-%04u%02u%02u-%02u%02u%02u-%03u-p%lu-t%lu.dmp",
            state.directory, now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute,
            now.wSecond, now.wMilliseconds, GetCurrentProcessId(), state.faultThread);
        if (pathResult < 0 || _snprintf_s(partial, sizeof(partial), _TRUNCATE, "%s.partial", finalPath) < 0)
        {
            Log("CRASH dump failed stage=path error=filename_too_long\r\n");
            SetEvent(state.completed);
            return 1;
        }
        HANDLE file = CreateFileA(partial, GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE)
        {
            const DWORD error = GetLastError();
            sprintf_s(line, "CRASH dump failed stage=create error=%lu path=%s\r\n", error, partial);
            Log(line);
            SetEvent(state.completed);
            return 1;
        }

        MINIDUMP_EXCEPTION_INFORMATION info{state.faultThread, &state.pointers, FALSE};
        const BOOL written = state.writeDump(GetCurrentProcess(), GetCurrentProcessId(), file,
            static_cast<MINIDUMP_TYPE>(MiniDumpNormal | MiniDumpWithDataSegs), &info, nullptr, nullptr);
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
        if (valid && !MoveFileA(partial, finalPath))
        {
            error = GetLastError();
            valid = false;
        }
        // Incomplete captures remain .partial evidence, never successful .dmp files.
        sprintf_s(line, "CRASH dump written=%d error=%lu bytes=%lld path=%s\r\n",
            valid ? 1 : 0, error, size.QuadPart, valid ? finalPath : partial);
        Log(line);
        SetEvent(state.completed);
        return valid ? 0 : 1;
    }

    inline LONG WINAPI Filter(EXCEPTION_POINTERS* pointers)
    {
        if (!pointers || !pointers->ExceptionRecord || !pointers->ContextRecord ||
            InterlockedCompareExchange(&state.claimed, 1, 0) != 0)
            return EXCEPTION_EXECUTE_HANDLER;
        // Preserve the original exception and thread. The failing stack is not
        // asked to format text, load DLLs, open files or call MiniDumpWriteDump.
        state.record = *pointers->ExceptionRecord;
        state.record.ExceptionRecord = nullptr;
        state.context = *pointers->ContextRecord;
        state.pointers.ExceptionRecord = &state.record;
        state.pointers.ContextRecord = &state.context;
        state.faultThread = GetCurrentThreadId();
        if (SetEvent(state.request))
            WaitForSingleObject(state.completed, 10000);
        return EXCEPTION_EXECUTE_HANDLER;
    }

    // Call at startup on the main thread. Other overflow-prone application
    // threads should call ReserveExceptionStack() on their own entry point.
    inline bool ReserveExceptionStack()
    {
        ULONG bytes = 64 * 1024;
        return SetThreadStackGuarantee(&bytes) != FALSE;
    }

    inline bool Install(const char* directory, LogSink log)
    {
        if (state.installed)
            return true;
        state.log = log;
        if (!directory || std::strlen(directory) >= sizeof(state.directory))
        {
            Log("CRASH capture unavailable stage=directory error=invalid_path\r\n");
            return false;
        }
        strcpy_s(state.directory, directory);
        if (!ReserveExceptionStack())
        {
            Log("CRASH capture unavailable stage=stack_guarantee\r\n");
            return false;
        }
        // Resolve dependencies before a crash, from the Windows system directory.
        state.dbghelp = LoadLibraryExW(L"dbghelp.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        DWORD initializeError = state.dbghelp ? ERROR_SUCCESS : GetLastError();
        if (state.dbghelp)
        {
            state.writeDump = reinterpret_cast<WriteDump>(GetProcAddress(state.dbghelp, "MiniDumpWriteDump"));
            if (!state.writeDump) initializeError = GetLastError();
        }
        state.request = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        if (!state.request) initializeError = GetLastError();
        state.completed = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        if (!state.completed) initializeError = GetLastError();
        HANDLE thread = nullptr;
        if (state.writeDump && state.request && state.completed)
        {
            thread = CreateThread(nullptr, 0, Worker, nullptr, 0, &state.workerThread);
            if (!thread) initializeError = GetLastError();
        }
        if (!thread)
        {
            char line[128]{};
            sprintf_s(line, "CRASH capture unavailable stage=initialize error=%lu\r\n", initializeError);
            Log(line);
            if (state.request) CloseHandle(state.request);
            if (state.completed) CloseHandle(state.completed);
            if (state.dbghelp) FreeLibrary(state.dbghelp);
            state.request = state.completed = nullptr;
            state.dbghelp = nullptr;
            state.writeDump = nullptr;
            return false;
        }
        CloseHandle(thread);
        state.installed = true;
        SetUnhandledExceptionFilter(Filter);
        Log("CRASH capture ready worker=dedicated stack_guarantee=65536 timeout_ms=10000\r\n");
        return true;
    }
}
