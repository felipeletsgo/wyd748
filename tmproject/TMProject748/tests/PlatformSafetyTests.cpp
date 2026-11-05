#include <winsock2.h>
#include <windows.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <thread>
#include <vector>
#include "../internal/platform/windows/CPSock.h"
#include "../internal/platform/network/ReceiveBuffer.h"
#include "../internal/platform/network/SocketEventPolicy.h"
#include "../internal/platform/windows/ComApartment.h"
#include "../internal/platform/windows/DirShowGraph.h"
#include "../internal/platform/windows/DisplayModeQuery.h"
#include "../internal/platform/diagnostics/LegacyLogFile.h"
#include "../internal/platform/diagnostics/ModuleSiblingPath.h"
#include "../internal/application/MixHelpRecord.h"
#include "../internal/ui/GuildMarkDownload.h"

// Platform boundary helpers called by CPSock, DirShow, NewApp, TMLog and the
// diagnostics log. Fakes replace Winsock, COM, display drivers and the CRT.
namespace
{
static_assert(socket_event::kRead == FD_READ && socket_event::kWrite == FD_WRITE &&
    socket_event::kConnect == FD_CONNECT && socket_event::kClose == FD_CLOSE,
    "socket event policy uses the Winsock notification values");

struct Counted
{
    long refs = 0;
    long releases = 0;
    void AddRef() { ++refs; }
    ULONG Release()
    {
        ++releases;
        return static_cast<ULONG>(--refs);
    }
};
struct __declspec(uuid("0b7f1a52-7f3e-4c1e-9a51-5a0000000001")) FakeControl : Counted {};
struct __declspec(uuid("0b7f1a52-7f3e-4c1e-9a51-5a0000000002")) FakeSeeking : Counted {};
struct __declspec(uuid("0b7f1a52-7f3e-4c1e-9a51-5a0000000003")) FakeAudio : Counted {};
struct __declspec(uuid("0b7f1a52-7f3e-4c1e-9a51-5a0000000004")) FakeEvents : Counted {};

struct FakeGraph
{
    int failAt = -1;
    int calls = 0;
    FakeControl control;
    FakeSeeking seeking;
    FakeAudio audio;
    FakeEvents events;

    HRESULT QueryInterface(REFIID iid, void** out)
    {
        *out = nullptr;
        if (calls++ == failAt)
            return E_NOINTERFACE;
        if (iid == __uuidof(FakeControl)) { control.AddRef(); *out = &control; }
        else if (iid == __uuidof(FakeSeeking)) { seeking.AddRef(); *out = &seeking; }
        else if (iid == __uuidof(FakeAudio)) { audio.AddRef(); *out = &audio; }
        else if (iid == __uuidof(FakeEvents)) { events.AddRef(); *out = &events; }
        else return E_NOINTERFACE;
        return S_OK;
    }
};

struct FakeFilter : Counted {};

struct FakeEnumerator
{
    std::vector<FakeFilter*> filters;
    std::size_t position = 0;
    HRESULT end = S_FALSE;
    bool endless = false;
    FakeFilter endlessFilter;
    int nextCalls = 0;

    HRESULT Reset()
    {
        position = 0;
        return S_OK;
    }
    HRESULT Next(ULONG, FakeFilter** out, ULONG* fetched)
    {
        ++nextCalls;
        if (endless)
        {
            endlessFilter.AddRef();
            *out = &endlessFilter;
            *fetched = 1;
            return S_OK;
        }
        if (position < filters.size())
        {
            *out = filters[position++];
            (*out)->AddRef();
            *fetched = 1;
            return S_OK;
        }
        *fetched = 0;
        return end;
    }
};

struct FakeFilterGraph
{
    int removed = 0;
    HRESULT RemoveFilter(FakeFilter* filter)
    {
        ++removed;
        --filter->refs; // the graph drops its own reference
        return S_OK;
    }
};

struct FakeEventQueue
{
    std::vector<long> codes;
    std::size_t position = 0;
    bool endless = false;
    int freed = 0;

    HRESULT GetEvent(long* code, LONG_PTR* first, LONG_PTR* second, long)
    {
        if (!endless && position >= codes.size())
            return E_ABORT;
        *code = endless ? 0x0E : codes[position++];
        *first = 1;
        *second = 2;
        return S_OK;
    }
    HRESULT FreeEventParams(long, LONG_PTR, LONG_PTR)
    {
        ++freed;
        return S_OK;
    }
};

struct FakeComApi
{
    inline static HRESULT result = S_OK;
    inline static int apartmentRefs = 0;
    inline static int uninitializeCalls = 0;
    inline static DWORD model = 0;

    static HRESULT Initialize(DWORD requested)
    {
        model = requested;
        if (SUCCEEDED(result))
            ++apartmentRefs;
        return result;
    }
    static void Uninitialize()
    {
        ++uninitializeCalls;
        --apartmentRefs;
    }
    static void Reset(HRESULT next)
    {
        result = next;
        apartmentRefs = 0;
        uninitializeCalls = 0;
        model = 0;
    }
};

struct FakeLogIo
{
    inline static int openResult = -1;
    inline static int opens = 0;
    inline static int commits = 0;
    inline static int closes = 0;
    inline static std::vector<int> writeDescriptors;
    inline static std::vector<unsigned int> writeSizes;

    static int Open(const char*)
    {
        ++opens;
        return openResult;
    }
    static int Write(int descriptor, const char*, unsigned int size)
    {
        writeDescriptors.push_back(descriptor);
        writeSizes.push_back(size);
        return static_cast<int>(size);
    }
    static int Commit(int)
    {
        ++commits;
        return 0;
    }
    static int Close(int)
    {
        ++closes;
        return 0;
    }
    static void Reset(int next)
    {
        openResult = next;
        opens = commits = closes = 0;
        writeDescriptors.clear();
        writeSizes.clear();
    }
};

// Same field layout as STRUCT_MIXHELP in BasedefStructs.h.
struct MixRecord
{
    short Color[9];
    short Icon;
    char Help[9][128];
    char Name[128];
};

struct LineSource
{
    std::vector<std::string> lines;
    std::size_t position = 0;

    // Follows fgets: at most capacity - 1 bytes, always terminated.
    bool operator()(char* buffer, int capacity)
    {
        if (position >= lines.size() || capacity <= 0)
            return false;
        const std::string& line = lines[position++];
        const std::size_t count = line.size() < static_cast<std::size_t>(capacity - 1)
            ? line.size() : static_cast<std::size_t>(capacity - 1);
        std::memcpy(buffer, line.data(), count);
        buffer[count] = '\0';
        return true;
    }
};

std::vector<std::string> MixRecordLines(const std::string& header, const std::string& firstRow)
{
    std::vector<std::string> lines{header, firstRow};
    while (lines.size() < static_cast<std::size_t>(mix_help::kRecordLines))
        lines.push_back("000000\n");
    return lines;
}
}

int RunPlatformSafetyTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) {
            ++failures;
            std::fprintf(stderr, "FAIL: %s\n", name);
        }
    };

    // A18-03 and A18-04: notification policy for the active socket.
    {
        using socket_event::Classify;
        using Step = socket_event::Step;
        check(Classify(false, true, false, FD_READ, 0) == Step::Ignore &&
            Classify(true, false, false, FD_CLOSE, 0) == Step::Ignore,
            "A18-03 events without the active socket are ignored");
        bool staleIgnored = true;
        for (const int event : {FD_READ, FD_WRITE, FD_CLOSE})
            for (const int error : {0, static_cast<int>(WSAECONNRESET)})
                staleIgnored = staleIgnored && Classify(true, true, true, event, error) == Step::Ignore;
        check(staleIgnored,
            "A18-03 a connecting stream ignores queued read, write and close events of a reused handle");
        check(Classify(true, true, true, FD_CONNECT, 0) == Step::Connected &&
            Classify(true, true, true, FD_CONNECT, WSAECONNREFUSED) == Step::Close,
            "A18-03 connection completion and connection failure still apply while connecting");
        check(Classify(true, true, false, FD_READ, 0) == Step::Read &&
            Classify(true, true, false, FD_WRITE, 0) == Step::Write &&
            Classify(true, true, false, FD_CONNECT, 0) == Step::Connected,
            "connected stream keeps read, write and connect handling");
        check(Classify(true, true, false, FD_CLOSE, 0) == Step::DrainThenClose,
            "A18-04 graceful close drains remaining bytes before the disconnect");
        check(Classify(true, true, false, FD_CLOSE, WSAECONNRESET) == Step::Close &&
            Classify(true, true, false, FD_READ, WSAENETDOWN) == Step::Close &&
            Classify(true, true, false, FD_WRITE, WSAENETDOWN) == Step::Close,
            "abortive close and packed errors disconnect without a drain");
        check(Classify(true, true, false, FD_OOB, 0) == Step::Ignore &&
            Classify(true, true, false, FD_OOB, WSAENETDOWN) == Step::Close,
            "unused notifications are ignored unless they carry an error");
    }

    // A18-05: a frame that ends at the receive window end stays readable to
    // the largest 16-bit structure size inside the same allocation.
    {
        const int capacity = RECV_BUFFER_SIZE;
        const std::size_t allocation = receive_buffer::AllocationSize(capacity);
        const std::size_t smallestFrameStart = static_cast<std::size_t>(capacity) - 12;
        check(allocation >= smallestFrameStart + 0xFFFF,
            "A18-05 a tail frame read as a 64 KiB structure stays inside the receive allocation");
        check(receive_buffer::AllocationSize(0) == 0 && receive_buffer::AllocationSize(-1) == 0,
            "A18-05 invalid receive capacities allocate nothing");
    }

    // A18-02: each failed interface query leaves defined pointers, and the
    // release path frees exactly the interfaces that were acquired.
    for (int failAt = -1; failAt < 4; ++failAt)
    {
        FakeGraph graph;
        graph.failAt = failAt;
        auto* control = reinterpret_cast<FakeControl*>(static_cast<std::uintptr_t>(0x11));
        auto* seeking = reinterpret_cast<FakeSeeking*>(static_cast<std::uintptr_t>(0x12));
        auto* audio = reinterpret_cast<FakeAudio*>(static_cast<std::uintptr_t>(0x13));
        auto* events = reinterpret_cast<FakeEvents*>(static_cast<std::uintptr_t>(0x14));
        const bool queried = dirshow_graph::QueryAll(graph, control, seeking, audio, events);
        const int acquired = failAt < 0 ? 4 : failAt;
        const bool defined =
            control == (acquired > 0 ? &graph.control : nullptr) &&
            seeking == (acquired > 1 ? &graph.seeking : nullptr) &&
            audio == (acquired > 2 ? &graph.audio : nullptr) &&
            events == (acquired > 3 ? &graph.events : nullptr);
        check(queried == (failAt < 0) && defined,
            "A18-02 a failed query step leaves every later interface null, never indeterminate");
        if (!defined)
            continue;
        dirshow_graph::ReleaseAll(events, audio, seeking, control);
        dirshow_graph::ReleaseAll(events, audio, seeking, control);
        check(!control && !seeking && !audio && !events &&
            graph.control.refs == 0 && graph.seeking.refs == 0 &&
            graph.audio.refs == 0 && graph.events.refs == 0 &&
            graph.control.releases == (acquired > 0 ? 1 : 0) &&
            graph.events.releases == (acquired > 3 ? 1 : 0),
            "A18-02 teardown releases each acquired interface once and nothing else");
    }

    // A18-02 and A18-11: the apartment reference balances only what it added,
    // and an application reference outlives a window reference.
    {
        FakeComApi::Reset(S_OK);
        {
            com_apartment::BasicReference<FakeComApi> reference(COINIT_APARTMENTTHREADED);
            check(reference.Owns() && FakeComApi::model == static_cast<DWORD>(COINIT_APARTMENTTHREADED),
                "first apartment initialization is owned");
        }
        check(FakeComApi::uninitializeCalls == 1 && FakeComApi::apartmentRefs == 0,
            "owned apartment is uninitialized once");

        FakeComApi::Reset(S_FALSE);
        {
            com_apartment::BasicReference<FakeComApi> reference(COINIT_APARTMENTTHREADED);
            check(reference.Owns(), "S_FALSE adds an apartment reference");
        }
        check(FakeComApi::uninitializeCalls == 1, "S_FALSE is balanced once");

        FakeComApi::Reset(RPC_E_CHANGED_MODE);
        {
            com_apartment::BasicReference<FakeComApi> reference(COINIT_APARTMENTTHREADED);
            check(!reference.Owns(), "a different apartment model is not owned");
        }
        check(FakeComApi::uninitializeCalls == 0,
            "A18-02 a failed initialization is never balanced by CoUninitialize");

        FakeComApi::Reset(S_OK);
        {
            com_apartment::BasicReference<FakeComApi> application(COINIT_APARTMENTTHREADED);
            FakeComApi::result = S_FALSE;
            {
                com_apartment::BasicReference<FakeComApi> storeWindow(COINIT_APARTMENTTHREADED);
            }
            check(FakeComApi::apartmentRefs == 1,
                "A18-11 closing the store window keeps the application apartment reference");
        }
        check(FakeComApi::apartmentRefs == 0, "application exit releases the last apartment reference");
    }

    // A18-09: enumeration continues only on S_OK, references are released once.
    for (const HRESULT end : {S_FALSE, E_INVALIDARG})
    {
        FakeFilter source, decoder, renderer;
        source.refs = decoder.refs = renderer.refs = 1; // graph references
        renderer.refs = 2; // another holder keeps the renderer
        FakeEnumerator enumerator;
        enumerator.filters = {&source, &decoder, &renderer};
        enumerator.end = end;
        auto filters = dirshow_graph::CollectFilters<FakeFilter>(enumerator);
        check(filters.size() == 3 && enumerator.nextCalls == 4,
            "A18-09 enumeration stops at S_FALSE or an error after the last filter");
        FakeFilterGraph graph;
        dirshow_graph::RemoveAndRelease(graph, filters);
        check(filters.empty() && graph.removed == 3 && source.refs == 0 && decoder.refs == 0 &&
            source.releases == 1 && decoder.releases == 1,
            "A18-09 each collected filter is removed and released once");
        check(renderer.refs == 1 && renderer.releases == 1,
            "A18-09 a filter with another holder is not forced to zero references");
    }
    {
        FakeEnumerator enumerator;
        enumerator.endless = true;
        auto filters = dirshow_graph::CollectFilters<FakeFilter>(enumerator);
        check(filters.size() == dirshow_graph::kMaxFilters &&
            enumerator.endlessFilter.refs == static_cast<long>(dirshow_graph::kMaxFilters),
            "A18-09 a faulty endless enumerator is bounded");
        FakeFilterGraph graph;
        enumerator.endlessFilter.refs += static_cast<long>(dirshow_graph::kMaxFilters);
        dirshow_graph::RemoveAndRelease(graph, filters);
        check(enumerator.endlessFilter.refs == 0, "A18-09 bounded collection releases what it holds");
    }
    {
        FakeEventQueue queue;
        queue.codes = {0x0E, dirshow_graph::kCompleteEvent, 0x0D};
        check(dirshow_graph::DrainEvents(queue) && queue.freed == 3 && queue.position == 3,
            "A18-09 every queued graph event is read and its parameters are freed");
        FakeEventQueue other;
        other.codes = {0x0E};
        check(!dirshow_graph::DrainEvents(other) && other.freed == 1,
            "A18-09 events without completion do not restart playback");
        FakeEventQueue endless;
        endless.endless = true;
        check(!dirshow_graph::DrainEvents(endless) &&
            endless.freed == dirshow_graph::kMaxEventsPerNotification,
            "A18-09 one notification reads a bounded number of events");
    }

    // C-36: every EnumDisplaySettings call receives dmSize and no driver extra.
    {
        struct Mode { DWORD width, height, bits; };
        const std::vector<Mode> modes{{800, 600, 32}, {1024, 768, 32}, {1280, 720, 32}};
        int calls = 0;
        bool prepared = true;
        const auto enumerate = [&](DWORD index, DEVMODEA* mode) -> BOOL {
            ++calls;
            prepared = prepared && mode->dmSize == sizeof(DEVMODEA) && mode->dmDriverExtra == 0 &&
                mode->dmPelsWidth == 0;
            // A driver may write output fields; the next query must reset them.
            mode->dmDriverExtra = 4096;
            mode->dmSize = 1;
            if (index >= modes.size())
                return FALSE;
            mode->dmPelsWidth = modes[index].width;
            mode->dmPelsHeight = modes[index].height;
            mode->dmBitsPerPel = modes[index].bits;
            return TRUE;
        };
        check(display_mode::HasMode<DEVMODEA>(enumerate, 1280, 720, 32) && calls == 3 && prepared,
            "C-36 each display mode query is prepared with dmSize and zero dmDriverExtra");
        calls = 0;
        check(!display_mode::HasMode<DEVMODEA>(enumerate, 1280, 720, 16) && calls == 4 && prepared,
            "C-36 an absent mode ends at the first failed query");
    }

    // C-32: descriptor state, truncated length and closed-log writes.
    {
        check(legacy_log::FormattedLength(-1, 1024) == 0 && legacy_log::FormattedLength(0, 1024) == 0 &&
            legacy_log::FormattedLength(5, 1024) == 5 && legacy_log::FormattedLength(1023, 1024) == 1023 &&
            legacy_log::FormattedLength(5000, 1024) == 1023 && legacy_log::FormattedLength(5, 0) == 0,
            "C-32 formatted length never exceeds the buffered bytes");
        const char buffer[1024]{};

        FakeLogIo::Reset(-1);
        legacy_log::File<FakeLogIo> failed;
        failed.Write(buffer, 10, sizeof(buffer));
        check(!failed.Open("clientlogs.log") && !failed.IsOpen() &&
            failed.Descriptor() == legacy_log::kClosedDescriptor,
            "C-32 a failed open leaves the log closed");
        failed.Write(buffer, 10, sizeof(buffer));
        failed.Close();
        check(FakeLogIo::writeSizes.empty() && FakeLogIo::commits == 0 && FakeLogIo::closes == 0,
            "C-32 writes and finalization never use a descriptor that was not opened");

        FakeLogIo::Reset(0);
        legacy_log::File<FakeLogIo> log;
        check(log.Open("clientlogs.log") && log.IsOpen() && log.Descriptor() == 0,
            "C-32 descriptor zero is a valid open log");
        log.Write(buffer, 5000, sizeof(buffer));
        check(FakeLogIo::writeSizes.size() == 1 && FakeLogIo::writeSizes[0] == sizeof(buffer) - 1 &&
            FakeLogIo::writeDescriptors[0] == 0 && FakeLogIo::commits == 1,
            "C-32 a long message writes only the formatted buffer bytes");
        log.Close();
        log.Close();
        log.Write(buffer, 10, sizeof(buffer));
        check(FakeLogIo::closes == 1 && FakeLogIo::commits == 2 && FakeLogIo::writeSizes.size() == 1 &&
            !log.IsOpen(), "C-32 finalization closes once and later writes are ignored");

        FakeLogIo::Reset(7);
        legacy_log::File<FakeLogIo> reopened;
        reopened.Open("a.log");
        reopened.Open("b.log");
        check(FakeLogIo::opens == 2 && FakeLogIo::closes == 1 && reopened.Descriptor() == 7,
            "C-32 reinitialization closes the previous descriptor first");
    }

    // C-08: the diagnostics path never reaches the CRT fail-fast handler.
    {
        char path[MAX_PATH]{};
        check(diagnostics_path::ReplaceFileName("C:\\Games\\WYD\\WYD.exe", "client-debug.log",
            path, sizeof(path)) && std::strcmp(path, "C:\\Games\\WYD\\client-debug.log") == 0,
            "C-08 the log is placed next to the executable");
        check(diagnostics_path::ReplaceFileName("C:\\Games\\WYD/WYD.exe", "a.log", path, sizeof(path)) &&
            std::strcmp(path, "C:\\Games\\WYD/a.log") == 0,
            "C-08 the last separator of either kind ends the directory");
        check(!diagnostics_path::ReplaceFileName("WYD.exe", "a.log", path, sizeof(path)) && path[0] == '\0',
            "C-08 a module path without a directory is rejected");
        std::string longModule = "C:\\";
        longModule.append(240, 'd');
        longModule += "\\WYD.exe";
        check(longModule.size() < MAX_PATH &&
            !diagnostics_path::ReplaceFileName(longModule.c_str(), "client-debug.log", path, sizeof(path)) &&
            path[0] == '\0',
            "C-08 a sibling path beyond MAX_PATH is rejected instead of failing fast");
        char exact[12]{};
        check(diagnostics_path::ReplaceFileName("C:\\d\\x.exe", "abcdef", exact, sizeof(exact)) &&
            std::strcmp(exact, "C:\\d\\abcdef") == 0 &&
            !diagnostics_path::ReplaceFileName("C:\\d\\x.exe", "abcdefg", exact, sizeof(exact)),
            "C-08 the bound accepts an exact fit and rejects one more byte");
    }

    // C-17: Mixhelp.dat indices, names and rows are data, bounded by the table.
    {
        std::vector<MixRecord> table(16);
        std::memset(table.data(), 0, table.size() * sizeof(MixRecord));
        LineSource source;
        const auto append = [&](const std::vector<std::string>& lines) {
            source.lines.insert(source.lines.end(), lines.begin(), lines.end());
        };
        append(MixRecordLines("Sword 3 7\n", "001234  \tFirst help\n"));
        append(MixRecordLines("Negative -1 7\n", "001111  \tmust not be written\n"));
        append(MixRecordLines("Outside 16 7\n", "001111  \tmust not be written\n"));
        append(MixRecordLines("Missing\n", "001111  \tmust not reuse index 3\n"));
        const std::string longText(200, 'h');
        append(MixRecordLines(std::string(200, 'n') + " 5 2\n", "004321  \t" + longText + "\n"));
        std::vector<std::string> shortRow{"Short 6 1\n", "001234  \tKept\n", "001234\n"};
        while (shortRow.size() < static_cast<std::size_t>(mix_help::kRecordLines))
            shortRow.push_back("000000\n");
        append(shortRow);
        const int applied = mix_help::Load(source, table.data(), static_cast<int>(table.size()));
        check(applied == 3, "C-17 only records with three fields and a valid index are applied");
        check(std::strcmp(table[3].Name, "Sword") == 0 && table[3].Icon == 7 &&
            table[3].Color[0] == 0x1234 && std::strcmp(table[3].Help[0], "First help") == 0,
            "C-17 a valid record keeps the established fields");
        bool untouched = true;
        for (const int index : {0, 1, 2, 4, 15})
            untouched = untouched && table[index].Name[0] == '\0' && table[index].Help[0][0] == '\0';
        check(untouched && std::strcmp(table[3].Help[0], "First help") == 0,
            "C-17 invalid, outside and incomplete headers write no record");
        check(std::strlen(table[5].Name) == sizeof(table[5].Name) - 1 &&
            std::strlen(table[5].Help[0]) == sizeof(table[5].Help[0]) - 1 &&
            table[5].Help[1][0] == '\0' && table[5].Color[0] == 0x4321,
            "C-17 a 200-byte name and help row are truncated inside their fields");
        check(std::strcmp(table[6].Help[0], "Kept") == 0 && table[6].Help[1][0] == '\0',
            "C-17 a row without the text column copies no stale bytes");
    }

    // C-12: worker completions reach the render thread once each.
    {
        auto& queue = guild_mark_download::CompletionQueue<int>::Shared();
        check(&queue == &guild_mark_download::CompletionQueue<int>::Shared(),
            "C-12 the completion queue is one process-lifetime instance");
        std::vector<int> values(200);
        for (int i = 0; i < static_cast<int>(values.size()); ++i)
            values[i] = i;
        std::vector<std::thread> workers;
        for (int worker = 0; worker < 4; ++worker)
            workers.emplace_back([&queue, &values, worker] {
                for (int i = worker; i < static_cast<int>(values.size()); i += 4)
                    queue.Push(&values[i]);
            });
        for (auto& worker : workers)
            worker.join();
        const auto done = queue.Drain();
        std::vector<int> seen(values.size());
        for (int* value : done)
            ++seen[static_cast<std::size_t>(*value)];
        bool once = done.size() == values.size();
        for (const int count : seen)
            once = once && count == 1;
        check(once && queue.Drain().empty(),
            "C-12 concurrent completions are drained exactly once");
    }

    return failures;
}
