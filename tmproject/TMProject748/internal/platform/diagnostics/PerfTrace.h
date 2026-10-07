#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>

struct IDirect3DDevice9;

// Opt-in client performance trace. Disabled unless the WYD_PERF_LOG
// environment variable names a CSV file ("{pid}" is replaced by the process
// id so several clients can trace at once). When disabled every hook returns
// after one cached flag test. When enabled the client writes one CSV row per
// second; gameplay, rendering and network behavior are unchanged.
namespace perf_trace {

enum class Phase : int { FrameMove = 0, Render, Present, CleanUp, Count };

// Counters accumulated over one sample window.
struct Interval
{
    std::uint32_t frames = 0;
    std::uint32_t hitches = 0; // frames slower than kHitchMs
    double phaseMs[static_cast<int>(Phase::Count)]{};
    double frameMs = 0.0;
    double maxFrameMs = 0.0;
    std::uint32_t drawCalls = 0;
    std::uint32_t primitives = 0;
    std::uint32_t setTexture = 0;
    std::uint32_t setRenderState = 0;
    std::uint32_t setTransform = 0;
    std::uint32_t effectsCreated = 0;
    std::uint32_t effectsDestroyed = 0;
    std::uint32_t trnLoads = 0;
    std::uint32_t datLoads = 0;
    double trnMs = 0.0;
    double datMs = 0.0;
};

// Scene and process state sampled at the end of a window.
struct Snapshot
{
    int sceneType = -1;
    std::uint32_t nodes = 0;
    std::uint32_t deletedNodes = 0;
    std::uint32_t effectNodes = 0;
    std::uint32_t humanNodes = 0;
    std::uint32_t itemNodes = 0;
    std::uint32_t groundObjectNodes = 0;
    int focused = 0;
    int minimized = 0;
    double workingSetMb = 0.0;
    double cpuPercent = 0.0; // of one logical core
};

constexpr double kHitchMs = 50.0;

inline const char* CsvHeader()
{
    return "t_s,fps,frame_ms,max_frame_ms,hitches,framemove_ms,render_ms,present_ms,"
           "cleanup_ms,draws_per_frame,prims_per_frame,settexture_per_frame,"
           "setrenderstate_per_frame,settransform_per_frame,effects_created,"
           "effects_destroyed,trn_loads,trn_ms,dat_loads,dat_ms,scene,nodes,"
           "deleted_nodes,effect_nodes,human_nodes,item_nodes,ground_object_nodes,"
           "focused,minimized,working_set_mb,cpu_pct\n";
}

// Formats one CSV row. Per-frame values are averages over the window;
// render_ms excludes Present, which is measured inside the render phase.
inline int FormatRow(char* buffer, std::size_t capacity, double elapsedSeconds,
    double windowSeconds, const Interval& in, const Snapshot& s)
{
    const double frames = in.frames ? static_cast<double>(in.frames) : 1.0;
    const auto avg = [&](Phase phase) { return in.phaseMs[static_cast<int>(phase)] / frames; };
    const double present = avg(Phase::Present);
    double render = avg(Phase::Render) - present;
    if (render < 0.0)
        render = 0.0;
    const double fps = windowSeconds > 0.0 ? in.frames / windowSeconds : 0.0;
    return std::snprintf(buffer, capacity,
        "%.1f,%.1f,%.2f,%.1f,%u,%.3f,%.3f,%.3f,%.3f,%.0f,%.0f,%.0f,%.0f,%.0f,%u,%u,"
        "%u,%.2f,%u,%.2f,%d,%u,%u,%u,%u,%u,%u,%d,%d,%.1f,%.1f\n",
        elapsedSeconds, fps, in.frameMs / frames, in.maxFrameMs, in.hitches,
        avg(Phase::FrameMove), render, present, avg(Phase::CleanUp),
        in.drawCalls / frames, in.primitives / frames, in.setTexture / frames,
        in.setRenderState / frames, in.setTransform / frames,
        in.effectsCreated, in.effectsDestroyed, in.trnLoads, in.trnMs,
        in.datLoads, in.datMs, s.sceneType, s.nodes, s.deletedNodes,
        s.effectNodes, s.humanNodes, s.itemNodes, s.groundObjectNodes,
        s.focused, s.minimized, s.workingSetMb, s.cpuPercent);
}

bool Enabled();
double NowMs();

// Counts device draw/state calls by patching the device vtable once.
void InstallDeviceHooks(IDirect3DDevice9* device);

void AddPhase(Phase phase, double ms);
void OnEffectCreated();
void OnEffectDestroyed();
void AddTerrainLoad(double ms);
void AddObjectLoad(double ms);
// Writes a "#load,<kind>,<file>,<ms>" detail line for one field file load.
void LogLoad(const char* kind, const char* file, double ms);

// Closes a frame. Returns true when a sample window has elapsed; the caller
// then fills a Snapshot and calls WriteSample.
bool EndFrame();
// Fills focus, minimized, working set and CPU fields for the given HWND.
void FillProcess(Snapshot& snapshot, void* window);
void WriteSample(const Snapshot& snapshot);

class ScopedPhase
{
public:
    explicit ScopedPhase(Phase phase)
        : m_phase(phase), m_start(Enabled() ? NowMs() : -1.0)
    {
    }
    ~ScopedPhase()
    {
        if (m_start >= 0.0)
            AddPhase(m_phase, NowMs() - m_start);
    }
    ScopedPhase(const ScopedPhase&) = delete;
    ScopedPhase& operator=(const ScopedPhase&) = delete;

private:
    Phase m_phase;
    double m_start;
};

enum class LoadKind { Terrain, Objects };

class ScopedLoad
{
public:
    ScopedLoad(LoadKind kind, const char* file)
        : m_kind(kind), m_file(file), m_start(Enabled() ? NowMs() : -1.0)
    {
    }
    ~ScopedLoad()
    {
        if (m_start < 0.0)
            return;
        const double ms = NowMs() - m_start;
        if (m_kind == LoadKind::Terrain)
            AddTerrainLoad(ms);
        else
            AddObjectLoad(ms);
        LogLoad(m_kind == LoadKind::Terrain ? "terrain" : "objects", m_file, ms);
    }
    ScopedLoad(const ScopedLoad&) = delete;
    ScopedLoad& operator=(const ScopedLoad&) = delete;

private:
    LoadKind m_kind;
    const char* m_file;
    double m_start;
};

} // namespace perf_trace
