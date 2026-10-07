#include "pch.h"
#include "PerfTrace.h"

#include <d3d9.h>
#include <psapi.h>
#include <cstdlib>

#pragma comment(lib, "psapi.lib")
#include <string>

namespace perf_trace {
namespace {

struct State
{
    bool initialized = false;
    bool enabled = false;
    std::FILE* file = nullptr;
    double frequencyMs = 0.0;
    double startMs = 0.0;
    double windowStartMs = 0.0;
    double lastFrameMs = -1.0;
    double lastCpuMs = -1.0;
    double lastCpuWallMs = 0.0;
    Interval interval;
};

State& GetState()
{
    static State state;
    return state;
}

double QueryMs(const State& state)
{
    LARGE_INTEGER counter{};
    QueryPerformanceCounter(&counter);
    return static_cast<double>(counter.QuadPart) / state.frequencyMs;
}

void Initialize(State& state)
{
    state.initialized = true;
    char path[MAX_PATH]{};
    const DWORD length = GetEnvironmentVariableA("WYD_PERF_LOG", path, MAX_PATH);
    if (length == 0 || length >= MAX_PATH)
        return;

    std::string target(path);
    const auto token = target.find("{pid}");
    if (token != std::string::npos)
        target.replace(token, 5, std::to_string(GetCurrentProcessId()));
    if (fopen_s(&state.file, target.c_str(), "w") != 0 || !state.file)
        return;

    LARGE_INTEGER frequency{};
    QueryPerformanceFrequency(&frequency);
    state.frequencyMs = static_cast<double>(frequency.QuadPart) / 1000.0;
    state.startMs = QueryMs(state);
    state.windowStartMs = state.startMs;
    std::fputs(CsvHeader(), state.file);
    std::fflush(state.file);
    state.enabled = true;
}

// IDirect3DDevice9 vtable slots (d3d9.h declaration order).
constexpr int kSetTransformSlot = 44;
constexpr int kSetRenderStateSlot = 57;
constexpr int kSetTextureSlot = 65;
constexpr int kDrawPrimitiveSlot = 81;
constexpr int kDrawIndexedPrimitiveSlot = 82;
constexpr int kDrawPrimitiveUPSlot = 83;
constexpr int kDrawIndexedPrimitiveUPSlot = 84;

using SetTransformFn = HRESULT(STDMETHODCALLTYPE*)(IDirect3DDevice9*, D3DTRANSFORMSTATETYPE, const D3DMATRIX*);
using SetRenderStateFn = HRESULT(STDMETHODCALLTYPE*)(IDirect3DDevice9*, D3DRENDERSTATETYPE, DWORD);
using SetTextureFn = HRESULT(STDMETHODCALLTYPE*)(IDirect3DDevice9*, DWORD, IDirect3DBaseTexture9*);
using DrawPrimitiveFn = HRESULT(STDMETHODCALLTYPE*)(IDirect3DDevice9*, D3DPRIMITIVETYPE, UINT, UINT);
using DrawIndexedPrimitiveFn = HRESULT(STDMETHODCALLTYPE*)(IDirect3DDevice9*, D3DPRIMITIVETYPE, INT, UINT, UINT, UINT, UINT);
using DrawPrimitiveUPFn = HRESULT(STDMETHODCALLTYPE*)(IDirect3DDevice9*, D3DPRIMITIVETYPE, UINT, const void*, UINT);
using DrawIndexedPrimitiveUPFn = HRESULT(STDMETHODCALLTYPE*)(IDirect3DDevice9*, D3DPRIMITIVETYPE, UINT, UINT, UINT, const void*, D3DFORMAT, const void*, UINT);

// The runtime may switch a device between several vtables (for example while
// recording a state block), so originals are kept per patched table.
enum Slot { kSetTransform, kSetRenderState, kSetTexture, kDrawPrimitive,
    kDrawIndexedPrimitive, kDrawPrimitiveUP, kDrawIndexedPrimitiveUP, kSlotCount };
constexpr int kSlotIndex[kSlotCount] = { kSetTransformSlot, kSetRenderStateSlot,
    kSetTextureSlot, kDrawPrimitiveSlot, kDrawIndexedPrimitiveSlot,
    kDrawPrimitiveUPSlot, kDrawIndexedPrimitiveUPSlot };

struct PatchedTable
{
    void** table = nullptr;
    void* original[kSlotCount]{};
};
PatchedTable s_tables[8];
IDirect3DDevice9* s_device = nullptr;

template <typename Fn>
Fn Original(IDirect3DDevice9* device, Slot slot)
{
    void** table = *reinterpret_cast<void***>(device);
    for (const auto& entry : s_tables)
        if (entry.table == table)
            return reinterpret_cast<Fn>(entry.original[slot]);
    for (const auto& entry : s_tables)
        if (entry.original[slot])
            return reinterpret_cast<Fn>(entry.original[slot]);
    return nullptr;
}

HRESULT STDMETHODCALLTYPE HookSetTransform(IDirect3DDevice9* device, D3DTRANSFORMSTATETYPE state, const D3DMATRIX* matrix)
{
    ++GetState().interval.setTransform;
    return Original<SetTransformFn>(device, kSetTransform)(device, state, matrix);
}

HRESULT STDMETHODCALLTYPE HookSetRenderState(IDirect3DDevice9* device, D3DRENDERSTATETYPE state, DWORD value)
{
    ++GetState().interval.setRenderState;
    return Original<SetRenderStateFn>(device, kSetRenderState)(device, state, value);
}

HRESULT STDMETHODCALLTYPE HookSetTexture(IDirect3DDevice9* device, DWORD stage, IDirect3DBaseTexture9* texture)
{
    ++GetState().interval.setTexture;
    return Original<SetTextureFn>(device, kSetTexture)(device, stage, texture);
}

HRESULT STDMETHODCALLTYPE HookDrawPrimitive(IDirect3DDevice9* device, D3DPRIMITIVETYPE type, UINT start, UINT count)
{
    auto& in = GetState().interval;
    ++in.drawCalls;
    in.primitives += count;
    return Original<DrawPrimitiveFn>(device, kDrawPrimitive)(device, type, start, count);
}

HRESULT STDMETHODCALLTYPE HookDrawIndexedPrimitive(IDirect3DDevice9* device, D3DPRIMITIVETYPE type,
    INT baseVertex, UINT minIndex, UINT vertices, UINT startIndex, UINT count)
{
    auto& in = GetState().interval;
    ++in.drawCalls;
    in.primitives += count;
    return Original<DrawIndexedPrimitiveFn>(device, kDrawIndexedPrimitive)(
        device, type, baseVertex, minIndex, vertices, startIndex, count);
}

HRESULT STDMETHODCALLTYPE HookDrawPrimitiveUP(IDirect3DDevice9* device, D3DPRIMITIVETYPE type, UINT count,
    const void* data, UINT stride)
{
    auto& in = GetState().interval;
    ++in.drawCalls;
    in.primitives += count;
    return Original<DrawPrimitiveUPFn>(device, kDrawPrimitiveUP)(device, type, count, data, stride);
}

HRESULT STDMETHODCALLTYPE HookDrawIndexedPrimitiveUP(IDirect3DDevice9* device, D3DPRIMITIVETYPE type,
    UINT minIndex, UINT vertices, UINT count, const void* indices, D3DFORMAT format,
    const void* data, UINT stride)
{
    auto& in = GetState().interval;
    ++in.drawCalls;
    in.primitives += count;
    return Original<DrawIndexedPrimitiveUPFn>(device, kDrawIndexedPrimitiveUP)(
        device, type, minIndex, vertices, count, indices, format, data, stride);
}

void* const kHooks[kSlotCount] = {
    reinterpret_cast<void*>(&HookSetTransform), reinterpret_cast<void*>(&HookSetRenderState),
    reinterpret_cast<void*>(&HookSetTexture), reinterpret_cast<void*>(&HookDrawPrimitive),
    reinterpret_cast<void*>(&HookDrawIndexedPrimitive), reinterpret_cast<void*>(&HookDrawPrimitiveUP),
    reinterpret_cast<void*>(&HookDrawIndexedPrimitiveUP) };

void PatchTable(void** table)
{
    PatchedTable* entry = nullptr;
    for (auto& candidate : s_tables)
    {
        if (candidate.table == table)
        {
            entry = &candidate;
            break;
        }
    }
    if (entry)
    {
        bool intact = true;
        for (int slot = 0; slot < kSlotCount; ++slot)
            intact = intact && table[kSlotIndex[slot]] == kHooks[slot];
        if (intact)
            return;
    }
    for (auto& candidate : s_tables)
    {
        if (entry)
            break;
        if (!candidate.table)
            entry = &candidate;
    }
    if (!entry)
        return;
    DWORD oldProtect = 0;
    const SIZE_T bytes = sizeof(void*) * (kDrawIndexedPrimitiveUPSlot + 1);
    if (!VirtualProtect(table, bytes, PAGE_EXECUTE_READWRITE, &oldProtect))
        return;
    entry->table = table;
    for (int slot = 0; slot < kSlotCount; ++slot)
    {
        void* current = table[kSlotIndex[slot]];
        if (current == kHooks[slot])
        {
            // A table copied from an already patched one: reuse a real original.
            current = nullptr;
            for (const auto& other : s_tables)
                if (&other != entry && other.original[slot])
                    current = other.original[slot];
            if (!current)
                continue;
        }
        entry->original[slot] = current;
        table[kSlotIndex[slot]] = kHooks[slot];
    }
    VirtualProtect(table, bytes, oldProtect, &oldProtect);
}

} // namespace

bool Enabled()
{
    State& state = GetState();
    if (!state.initialized)
        Initialize(state);
    return state.enabled;
}

double NowMs()
{
    return Enabled() ? QueryMs(GetState()) : 0.0;
}

void InstallDeviceHooks(IDirect3DDevice9* device)
{
    if (!device || !Enabled())
        return;
    s_device = device;
    PatchTable(*reinterpret_cast<void***>(device));
}

void AddPhase(Phase phase, double ms)
{
    if (Enabled())
        GetState().interval.phaseMs[static_cast<int>(phase)] += ms;
}

void OnEffectCreated()
{
    if (Enabled())
        ++GetState().interval.effectsCreated;
}

void OnEffectDestroyed()
{
    if (Enabled())
        ++GetState().interval.effectsDestroyed;
}

void AddTerrainLoad(double ms)
{
    if (!Enabled())
        return;
    auto& in = GetState().interval;
    ++in.trnLoads;
    in.trnMs += ms;
}

void AddObjectLoad(double ms)
{
    if (!Enabled())
        return;
    auto& in = GetState().interval;
    ++in.datLoads;
    in.datMs += ms;
}

void LogLoad(const char* kind, const char* file, double ms)
{
    if (!Enabled())
        return;
    State& state = GetState();
    std::fprintf(state.file, "#load,%s,%s,%.2f\n", kind ? kind : "", file ? file : "", ms);
}

bool EndFrame()
{
    if (!Enabled())
        return false;
    State& state = GetState();
    // The D3D9 runtime rewrites the device's vtable copy; restore the hooks.
    if (s_device)
        PatchTable(*reinterpret_cast<void***>(s_device));
    const double now = QueryMs(state);
    if (state.lastFrameMs >= 0.0)
    {
        const double frame = now - state.lastFrameMs;
        auto& in = state.interval;
        ++in.frames;
        in.frameMs += frame;
        if (frame > in.maxFrameMs)
            in.maxFrameMs = frame;
        if (frame > kHitchMs)
            ++in.hitches;
    }
    state.lastFrameMs = now;
    return now - state.windowStartMs >= 1000.0;
}

double ProcessCpuMs()
{
    FILETIME created{}, exited{}, kernel{}, user{};
    if (!GetProcessTimes(GetCurrentProcess(), &created, &exited, &kernel, &user))
        return 0.0;
    const auto toMs = [](const FILETIME& time) {
        ULARGE_INTEGER value{};
        value.LowPart = time.dwLowDateTime;
        value.HighPart = time.dwHighDateTime;
        return static_cast<double>(value.QuadPart) / 10000.0;
    };
    return toMs(kernel) + toMs(user);
}

void FillProcess(Snapshot& snapshot, void* window)
{
    if (!Enabled())
        return;
    State& state = GetState();
    const HWND hwnd = static_cast<HWND>(window);
    snapshot.focused = hwnd && GetForegroundWindow() == hwnd ? 1 : 0;
    snapshot.minimized = hwnd && IsIconic(hwnd) ? 1 : 0;

    PROCESS_MEMORY_COUNTERS memory{};
    if (GetProcessMemoryInfo(GetCurrentProcess(), &memory, sizeof(memory)))
        snapshot.workingSetMb = static_cast<double>(memory.WorkingSetSize) / (1024.0 * 1024.0);

    const double cpu = ProcessCpuMs();
    const double wall = QueryMs(state);
    if (state.lastCpuMs >= 0.0 && wall > state.lastCpuWallMs)
        snapshot.cpuPercent = 100.0 * (cpu - state.lastCpuMs) / (wall - state.lastCpuWallMs);
    state.lastCpuMs = cpu;
    state.lastCpuWallMs = wall;
}

void WriteSample(const Snapshot& snapshot)
{
    if (!Enabled())
        return;
    State& state = GetState();
    const double now = QueryMs(state);
    char row[512]{};
    const int length = FormatRow(row, sizeof(row), (now - state.startMs) / 1000.0,
        (now - state.windowStartMs) / 1000.0, state.interval, snapshot);
    if (length > 0)
    {
        std::fputs(row, state.file);
        std::fflush(state.file);
    }
    state.interval = Interval{};
    state.windowStartMs = now;
}

} // namespace perf_trace
