#include "../internal/platform/diagnostics/CrashCapture.h"
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
        const std::string log = Read(directory + "\\capture.log");
        Check(log.find("CRASH capture ready") != std::string::npos, "worker initialized before the fault");
        Check(log.find("CRASH code=") != std::string::npos, "original exception logged");

        WIN32_FIND_DATAA data{};
        HANDLE search = FindFirstFileA((directory + "\\*.dmp").c_str(), &data);
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
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    if (argc == 4 && std::strcmp(argv[1], "--child") == 0)
    {
        logPath = std::string(argv[2]) + "\\capture.log";
        const bool failure = std::strcmp(argv[3], "write-failure") == 0;
        const std::string output = failure ? std::string(argv[2]) + "\\missing\\nested" : argv[2];
        if (!wyd_crash::Install(output.c_str(), Log)) return 2;
        // Installation is idempotent and does not start another worker.
        if (!wyd_crash::Install(output.c_str(), Log)) return 3;
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
        if (std::strcmp(argv[3], "overflow") == 0) Overflow(0);
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
    std::printf("CrashCaptureTests: %d checks, %d failures; evidence=%s\n", checks, failures, root.c_str());
    return failures == 0 ? 0 : 1;
}
