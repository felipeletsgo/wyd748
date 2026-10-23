#include "../internal/platform/diagnostics/CrashCapture.h"
#include "../internal/platform/diagnostics/CrashMonitor.h"
#include "../internal/ui/RenderControlList.h"
#include <intrin.h>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace
{
    std::string logPath;
    int checks = 0;
    int failures = 0;

    void Check(bool condition, const char* message)
    {
        ++checks;
        if (!condition)
        {
            ++failures;
            std::printf("FAIL: %s\n", message);
        }
    }

    void Log(const char* message)
    {
        HANDLE file = CreateFileA(logPath.c_str(), FILE_APPEND_DATA, FILE_SHARE_READ,
            nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file != INVALID_HANDLE_VALUE)
        {
            DWORD bytes = 0;
            WriteFile(file, message, static_cast<DWORD>(std::strlen(message)), &bytes, nullptr);
            CloseHandle(file);
        }
    }

    using Recurse = void (*)(unsigned int);
    void Overflow(unsigned int depth);
    Recurse volatile recurse = Overflow;
    volatile unsigned int consumed = 0;

    BOOL WINAPI RejectDump(HANDLE, DWORD, HANDLE, MINIDUMP_TYPE,
        const MINIDUMP_EXCEPTION_INFORMATION*, const MINIDUMP_USER_STREAM_INFORMATION*,
        const MINIDUMP_CALLBACK_INFORMATION*)
    {
        SetLastError(ERROR_DISK_FULL);
        return FALSE;
    }

    __declspec(noinline) void Overflow(unsigned int depth)
    {
        volatile char page[4096]{};
        page[depth % sizeof(page)] = static_cast<char>(depth);
        recurse(depth + 1);
        consumed += static_cast<unsigned int>(page[depth % sizeof(page)]);
    }

    DWORD WINAPI CrashThread(void*)
    {
        if (!wyd_crash::ReserveExceptionStack()) return 7;
        char line[128]{};
        sprintf_s(line, "TEST fault_tid=%lu\r\n", GetCurrentThreadId());
        Log(line);
        Overflow(0);
        return 8;
    }

    std::string Read(const std::string& path)
    {
        std::ifstream file(path, std::ios::binary);
        return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }

    void HandledException()
    {
        __try { RaiseException(0xE1234567, 0, 0, nullptr); }
        __except (EXCEPTION_EXECUTE_HANDLER) { }
    }

    int GuardedGeometryLifecycle()
    {
        struct Geometry { Geometry* m_pNextGeom = nullptr; };
        struct List { Geometry* pHeadGeom = nullptr; Geometry* pTailGeom = nullptr; };
        auto* dying = static_cast<Geometry*>(VirtualAlloc(nullptr, 4096, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
        if (!dying) return 16;
        dying->m_pNextGeom = nullptr;
        List list;
        Geometry first, replacement;
        if (!render_control_list::Append(list, &first) || !render_control_list::Append(list, dying) ||
            !render_control_list::Remove(list, dying)) return 17;
        // A stale tail now faults deterministically, rather than depending on
        // whether the allocator happens to reuse the just-destroyed icon.
        DWORD previous = 0;
        if (!VirtualProtect(dying, 4096, PAGE_NOACCESS, &previous)) return 18;
        const bool valid = render_control_list::Append(list, &replacement) &&
            first.m_pNextGeom == &replacement && list.pTailGeom == &replacement;
        VirtualFree(dying, 0, MEM_RELEASE);
        return valid ? 0 : 19;
    }

    // A heap block whose only reference is a stack local of the faulting
    // frame. Its text exists nowhere else, so finding it proves the dump kept
    // indirectly referenced heap memory (the objects a heap failure needs).
    std::string ReferencedMarker(DWORD pid)
    {
        char text[64]{};
        sprintf_s(text, "wyd-referenced-%lu-%lu", pid, pid ^ 0x5A5A1234u);
        return text;
    }

    char* PlantReferencedMarker()
    {
        auto* block = static_cast<char*>(HeapAlloc(GetProcessHeap(), 0, 64));
        if (block)
            sprintf_s(block, 64, "wyd-referenced-%lu-%lu", GetCurrentProcessId(), GetCurrentProcessId() ^ 0x5A5A1234u);
        return block;
    }

    void RunChild(const char* executable, const std::string& directory, const char* mode, DWORD expected)
    {
        Check(CreateDirectoryA(directory.c_str(), nullptr) != FALSE, "create isolated capture directory");
        std::string command = std::string("\"") + executable + "\" --child \"" + directory + "\" " + mode;
        STARTUPINFOA startup{};
        startup.cb = sizeof(startup);
        PROCESS_INFORMATION child{};
        const BOOL launched = CreateProcessA(executable, command.data(), nullptr, nullptr, FALSE,
            CREATE_NO_WINDOW, nullptr, nullptr, &startup, &child);
        Check(launched != FALSE, "launch console fault child (not the game)");
        if (!launched) return;
        const DWORD wait = WaitForSingleObject(child.hProcess, 20000);
        Check(wait == WAIT_OBJECT_0, "crash child terminates within timeout");
        if (wait != WAIT_OBJECT_0)
        {
            // Only this test's own timed-out subprocess is terminated.
            TerminateProcess(child.hProcess, 9);
            WaitForSingleObject(child.hProcess, 5000);
        }
        DWORD exitCode = 0;
        GetExitCodeProcess(child.hProcess, &exitCode);
        Check(exitCode == expected, "child retains the original exception exit code");
        CloseHandle(child.hThread);
        CloseHandle(child.hProcess);
        std::string log = Read(directory + "\\capture.log");
        const bool external = std::strncmp(mode, "external-", 9) == 0;
        if (external)
        {
            Check(log.find("CRASH monitor started=1") != std::string::npos, "external monitor attached before fault");
            log += Read(directory + "\\crash-monitor-p" + std::to_string(child.dwProcessId) + ".log");
            if (expected != 0)
                Check(log.find("capture_pid=") != std::string::npos, "exception captured outside the damaged process");
        }
        Check(log.find("CRASH capture ready") != std::string::npos, "worker initialized before the fault");
        Check((log.find("CRASH code=") != std::string::npos) == (expected != 0), "only unhandled exceptions are captured");

        WIN32_FIND_DATAA data{};
        HANDLE search = FindFirstFileA((directory + "\\*.dmp").c_str(), &data);
        if (expected == 0)
        {
            Check(search == INVALID_HANDLE_VALUE, "normal exit and handled first-chance exceptions do not produce dumps");
            if (search != INVALID_HANDLE_VALUE) FindClose(search);
            return;
        }
        if (std::strcmp(mode, "write-failure") == 0 || std::strcmp(mode, "dump-failure") == 0)
        {
            Check(search == INVALID_HANDLE_VALUE, "failed write does not publish an empty dump");
            if (std::strcmp(mode, "write-failure") == 0)
                Check(log.find("stage=create error=3") != std::string::npos, "missing directory error logged");
            else
            {
                Check(log.find("dump written=0 error=112") != std::string::npos, "writer failure logged with original error");
                WIN32_FIND_DATAA partialData{};
                HANDLE partial = FindFirstFileA((directory + "\\*.partial").c_str(), &partialData);
                Check(partial != INVALID_HANDLE_VALUE, "failed dump retained only as partial evidence");
                if (partial != INVALID_HANDLE_VALUE) FindClose(partial);
            }
            Check(log.find("dump written=1") == std::string::npos, "failure never reports success");
            if (search != INVALID_HANDLE_VALUE) FindClose(search);
            return;
        }
        Check(search != INVALID_HANDLE_VALUE, "real minidump saved");
        if (search == INVALID_HANDLE_VALUE) return;
        const std::string bytes = Read(directory + "\\" + data.cFileName);
        Check(FindNextFileA(search, &data) == FALSE, "exactly one dump per fault");
        FindClose(search);
        Check(bytes.size() >= sizeof(MINIDUMP_HEADER), "dump is not empty");
        if (bytes.size() < sizeof(MINIDUMP_HEADER)) return;
        // Real faults for both writers. The synthetic heap-status context comes
        // from RtlCaptureContext, which on x86 without frame pointers reports
        // an outer frame, so main's locals lie outside its dumped stack.
        if (std::strcmp(mode, "access") == 0 || std::strcmp(mode, "external-access") == 0)
            Check(bytes.find(ReferencedMarker(child.dwProcessId)) != std::string::npos,
                "dump keeps heap memory referenced from the faulting stack");
        // Copy to aligned writable storage for the Windows minidump reader.
        std::vector<unsigned long long> aligned((bytes.size() + 7) / 8);
        std::memcpy(aligned.data(), bytes.data(), bytes.size());
        void* dump = aligned.data();
        Check(static_cast<MINIDUMP_HEADER*>(dump)->Signature == MINIDUMP_SIGNATURE, "valid minidump signature");
        PMINIDUMP_DIRECTORY streamDirectory = nullptr;
        void* stream = nullptr;
        ULONG length = 0;
        const BOOL hasException = MiniDumpReadDumpStream(dump, ExceptionStream, &streamDirectory, &stream, &length);
        Check(hasException && length >= sizeof(MINIDUMP_EXCEPTION_STREAM), "exception stream exists");
        if (hasException && length >= sizeof(MINIDUMP_EXCEPTION_STREAM))
        {
            auto exception = static_cast<MINIDUMP_EXCEPTION_STREAM*>(stream);
            Check(exception->ExceptionRecord.ExceptionCode == expected, "dump preserves original exception code");
            const auto marker = log.find("TEST fault_tid=");
            const DWORD faultTid = marker == std::string::npos ? 0 :
                std::strtoul(log.c_str() + marker + std::strlen("TEST fault_tid="), nullptr, 10);
            Check(exception->ThreadId == faultTid && faultTid != 0, "dump identifies the faulting thread");
            const auto workerMarker = log.find("capture_tid=");
            const DWORD workerTid = workerMarker == std::string::npos ? 0 :
                std::strtoul(log.c_str() + workerMarker + std::strlen("capture_tid="), nullptr, 10);
            Check(workerTid != 0 && workerTid != faultTid, "dump written on a separate healthy thread");
            const auto location = exception->ThreadContext;
            const bool contextValid = location.DataSize >= sizeof(CONTEXT) &&
                static_cast<unsigned long long>(location.Rva) + location.DataSize <= bytes.size();
            Check(contextValid, "original thread context saved");
            if (contextValid)
            {
                auto context = reinterpret_cast<const CONTEXT*>(bytes.data() + location.Rva);
                Check(context->Eip == exception->ExceptionRecord.ExceptionAddress, "context points to original fault");
                Check(context->Esp != 0, "original stack pointer saved");
            }
        }
        Check(MiniDumpReadDumpStream(dump, ModuleListStream, &streamDirectory, &stream, &length) != FALSE,
            "module list available for symbolization");
        Check(MiniDumpReadDumpStream(dump, ThreadListStream, &streamDirectory, &stream, &length) != FALSE,
            "thread stacks available for analysis");
        Check(log.find("dump written=1 error=0") != std::string::npos, "successful durable capture logged");
        Check(log.find("module=") != std::string::npos && log.find("rva=0x") != std::string::npos,
            "ASLR module and relative address logged");
    }
}

int main(int argc, char** argv)
{
    const int monitorResult = wyd_crash_monitor::RunFromCommandLine();
    if (monitorResult >= 0) return monitorResult;
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    if (argc == 4 && std::strcmp(argv[1], "--child") == 0)
    {
        logPath = std::string(argv[2]) + "\\capture.log";
        const bool failure = std::strcmp(argv[3], "write-failure") == 0;
        const std::string output = failure ? std::string(argv[2]) + "\\missing\\nested" : argv[2];
        if (!wyd_crash::Install(output.c_str(), Log)) return 2;
        // Installation is idempotent and does not start another worker.
        if (!wyd_crash::Install(output.c_str(), Log)) return 3;
        if (std::strncmp(argv[3], "external-", 9) == 0 && !wyd_crash_monitor::Start(output.c_str(), Log)) return 15;
        if (std::strcmp(argv[3], "dump-failure") == 0)
            wyd_crash::state.writeDump = RejectDump;
        if (std::strcmp(argv[3], "thread-overflow") == 0)
        {
            HANDLE thread = CreateThread(nullptr, 0, CrashThread, nullptr, 0, nullptr);
            if (!thread) return 4;
            WaitForSingleObject(thread, INFINITE);
            return 5;
        }
        char line[128]{};
        sprintf_s(line, "TEST fault_tid=%lu\r\n", GetCurrentThreadId());
        Log(line);
        char* volatile referenced = PlantReferencedMarker();
        if (!referenced) return 20;
        if (std::strcmp(argv[3], "external-handled") == 0)
        {
            HandledException();
            return 0;
        }
        if (std::strcmp(argv[3], "geometry-lifecycle") == 0) return GuardedGeometryLifecycle();
        if (std::strcmp(argv[3], "external-normal") == 0) return 0;
        if (std::strcmp(argv[3], "overflow") == 0 || std::strcmp(argv[3], "external-overflow") == 0) Overflow(0);
        if (std::strcmp(argv[3], "external-fastfail") == 0) __fastfail(7);
        if (std::strcmp(argv[3], "external-heap-status") == 0)
        {
            // Exercise the exact fatal status and bypass path. This is not a
            // claim to reproduce the game's still-unidentified heap write.
            CONTEXT context{};
            RtlCaptureContext(&context);
            EXCEPTION_RECORD record{};
            record.ExceptionCode = 0xC0000374;
            record.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
            record.ExceptionAddress = reinterpret_cast<void*>(context.Eip);
            RaiseFailFastException(&record, &context, 0);
        }
        volatile ULONG_PTR badAddress = 1;
        *reinterpret_cast<volatile int*>(badAddress) = 42;
        return 6;
    }
    char executable[MAX_PATH]{};
    GetModuleFileNameA(nullptr, executable, MAX_PATH);
    std::string root = executable;
    root.resize(root.find_last_of('\\'));
    root += "\\crash-capture-tests-" + std::to_string(GetCurrentProcessId()) + "-" + std::to_string(GetTickCount64());
    Check(CreateDirectoryA(root.c_str(), nullptr) != FALSE, "create build-only test artifact root");
    RunChild(executable, root + "\\overflow", "overflow", EXCEPTION_STACK_OVERFLOW);
    RunChild(executable, root + "\\thread-overflow", "thread-overflow", EXCEPTION_STACK_OVERFLOW);
    RunChild(executable, root + "\\access", "access", EXCEPTION_ACCESS_VIOLATION);
    RunChild(executable, root + "\\write-failure", "write-failure", EXCEPTION_ACCESS_VIOLATION);
    RunChild(executable, root + "\\dump-failure", "dump-failure", EXCEPTION_ACCESS_VIOLATION);
    RunChild(executable, root + "\\external-access", "external-access", EXCEPTION_ACCESS_VIOLATION);
    RunChild(executable, root + "\\external-overflow", "external-overflow", EXCEPTION_STACK_OVERFLOW);
    RunChild(executable, root + "\\external-fastfail", "external-fastfail", 0xC0000409);
    RunChild(executable, root + "\\external-heap-status", "external-heap-status", 0xC0000374);
    RunChild(executable, root + "\\external-normal", "external-normal", 0);
    RunChild(executable, root + "\\external-handled", "external-handled", 0);
    RunChild(executable, root + "\\geometry-lifecycle", "geometry-lifecycle", 0);
    std::printf("CrashCaptureTests: %d checks, %d failures; evidence=%s\n", checks, failures, root.c_str());
    return failures == 0 ? 0 : 1;
}
