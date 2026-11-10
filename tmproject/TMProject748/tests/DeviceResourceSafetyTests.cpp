#include "../internal/platform/windows/DeviceFrame.h"
#include "../internal/platform/windows/DeviceReset.h"
#include "../internal/platform/windows/DeviceModeFamily.h"
#include <d3d9.h>
#include "../internal/platform/windows/AdapterQuirks.h"
#include "../internal/platform/windows/FrameRateSampler.h"
#include "../internal/platform/windows/DesktopGamma.h"
#include "../internal/platform/windows/SoundPlayback.h"
#include "../internal/platform/windows/WaveFormatChunk.h"
#include "../internal/render/resources/ModelTextureDirectory.h"
#include "../internal/render/mesh/MsaLoad.h"
#include "../internal/render/mesh/SkinMeshLoad.h"
#include "../internal/render/mesh/SkinAnimationIndex.h"
#include "../internal/render/mesh/FallbackCostumeTable.h"
#include "../internal/render/mesh/ScopeRestore.h"
#include <dsound.h>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {
// Direct3D 9 rule: Reset fails with D3DERR_INVALIDCALL while any DEFAULT-pool
// object is alive. MANAGED objects survive Reset.
struct DeviceModel {
    int defaultLive = 0;
    HRESULT cooperative = D3DERR_DEVICENOTRESET;
    int resets = 0;
    HRESULT TestCooperativeLevel() const { return cooperative; }
    HRESULT BeginScene() { return S_OK; }
    HRESULT Reset()
    {
        ++resets;
        if (defaultLive != 0) return D3DERR_INVALIDCALL;
        cooperative = S_OK;
        return S_OK;
    }
};

struct PoolObject {
    DeviceModel* device = nullptr;
    device_reset::Pool pool = device_reset::Pool::Managed;
    int references = 1;
    std::string* order = nullptr;
    char tag = '?';
    ULONG Release()
    {
        if (order) *order += tag;
        if (--references == 0 && device && pool == device_reset::Pool::Default) --device->defaultLive;
        return static_cast<ULONG>(references);
    }
};

PoolObject* CreateDefault(DeviceModel& device, std::string* order, char tag)
{
    ++device.defaultLive;
    return new PoolObject{&device, device_reset::Pool::Default, 1, order, tag};
}

// A cache entry and a detached borrower, as MeshManager entries and the skin
// parts of cached fields, mantles, mounts or delayed-delete actors.
struct ManagedMesh {
    bool alive = true;
};

struct Buffer {
    HRESULT result = S_OK;
    DWORD status = 0;
    HRESULT GetStatus(DWORD* value) const
    {
        if (SUCCEEDED(result) && value) *value = status;
        return result;
    }
};

struct Row {
    char szFileName[255];
};

std::filesystem::path FindRuntimeFrom(std::filesystem::path current)
{
    for (int depth = 0; depth < 8 && !current.empty(); ++depth) {
        if (std::filesystem::is_regular_file(current / "tmproject/client748/Mesh/MeshList.txt"))
            return current / "tmproject/client748";
        if (current == current.parent_path()) break;
        current = current.parent_path();
    }
    return {};
}

// Searches upward from the executable, then from the working directory.
std::filesystem::path FindRuntime()
{
    wchar_t executable[MAX_PATH]{};
    const DWORD length = GetModuleFileNameW(nullptr, executable, MAX_PATH);
    if (length != 0 && length < MAX_PATH) {
        auto runtime = FindRuntimeFrom(std::filesystem::path(executable).parent_path());
        if (!runtime.empty()) return runtime;
    }
    return FindRuntimeFrom(std::filesystem::current_path());
}

std::vector<char> ReadAll(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary);
    return std::vector<char>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

template<class T>
bool ReadAt(const std::vector<char>& data, std::size_t& offset, T& value)
{
    if (offset > data.size() || data.size() - offset < sizeof(T)) return false;
    std::memcpy(&value, data.data() + offset, sizeof(T));
    offset += sizeof(T);
    return true;
}

// Applies the production .msa limits in the loader's order to one file image.
bool ShippedMsaPasses(const std::vector<char>& data)
{
    std::size_t offset = 0;
    std::uint32_t header[3]{};
    if (!ReadAt(data, offset, header) || !msa_load::ValidHeader(header[0], header[1], header[2])) return false;
    msa_load::Range ranges[msa_load::MaxAttributes]{};
    for (std::uint32_t i = 0; i < header[2]; ++i)
        if (!ReadAt(data, offset, ranges[i])) return false;
    if (data.size() - offset < header[2] * msa_load::TextureFieldBytes) return false;
    offset += header[2] * msa_load::TextureFieldBytes;
    std::uint32_t indexBytes = 0;
    if (!ReadAt(data, offset, indexBytes) || !msa_load::ValidIndexBytes(indexBytes, data.size() - offset)) return false;
    offset += indexBytes;
    std::uint32_t vertexBytes = 0;
    msa_load::VertexPlan plan;
    if (!ReadAt(data, offset, vertexBytes) ||
        !msa_load::PlanVertices(header[0], header[1], vertexBytes, data.size() - offset, plan)) return false;
    return msa_load::RangesFit(ranges, header[2], plan.vertices, indexBytes / 2);
}
}

int RunDeviceResourceSafetyTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) {
            ++failures;
            std::fprintf(stderr, "FAIL: DeviceResourceSafety: %s\n", name);
        }
    };

    // C-29 + A23-01: recovery releases DEFAULT-pool targets only. Reset then
    // succeeds and every borrowed MANAGED mesh is still valid afterwards.
    {
        DeviceModel device;
        std::string order;
        PoolObject* texture = CreateDefault(device, &order, 'T');
        PoolObject* surface = CreateDefault(device, &order, 'S');
        ManagedMesh cacheEntry;
        ManagedMesh* borrowed = &cacheEntry;
        bool lost = true;
        device_frame::Recovery recovery;
        DWORD now = 0;
        const auto attempt = [&](bool releaseTargets) {
            return device_frame::Begin(&device, lost, true, recovery,
                []() -> HRESULT { return S_OK; },
                [&]() -> HRESULT {
                    if (releaseTargets) device_reset::ReleaseTarget(texture, surface);
                    return device.Reset();
                },
                []() -> HRESULT { return S_OK; },
                [&] { return now; });
        };
        check(!attempt(false) && lost && device.resets == 1, "live DEFAULT target blocks Reset (C-29 trigger)");
        check(!attempt(true) && !lost && device.defaultLive == 0, "released targets let Reset succeed");
        check(order == "ST" && !texture && !surface, "surface released before its texture; pointers cleared");
        check(borrowed->alive, "recovery never frees a borrowed managed mesh (A23-01)");
        check(attempt(false) && recovery.failures == 0 && !recovery.fatal, "drawing resumes after recovery");
        device_reset::ReleaseTarget(texture, surface);
        check(device.defaultLive == 0 && order == "ST", "repeated release is harmless");
        check(device_reset::ReleaseBeforeReset(device_reset::Pool::Default) &&
            !device_reset::ReleaseBeforeReset(device_reset::Pool::Managed) &&
            !device_reset::ReleaseBeforeReset(device_reset::Pool::SystemMemory),
            "only DEFAULT pool is released before Reset");
        PoolObject* textures[2]{CreateDefault(device, nullptr, 'a'), CreateDefault(device, nullptr, 'b')};
        PoolObject* surfaces[2]{nullptr, CreateDefault(device, nullptr, 'c')};
        device_reset::ReleaseTargets(textures, surfaces);
        check(device.defaultLive == 0 && !textures[0] && !textures[1] && !surfaces[1], "target arrays release every slot");
    }

    // A18-12: a cooperative-level status other than loss or pending reset is
    // terminal; repeated restore failures become terminal after a bound.
    {
        DeviceModel device;
        device.cooperative = D3DERR_DRIVERINTERNALERROR;
        bool lost = true;
        device_frame::Recovery recovery;
        int callbacks = 0;
        const auto begin = [&] {
            return device_frame::Begin(&device, lost, true, recovery,
                [&]() -> HRESULT { ++callbacks; return S_OK; },
                [&]() -> HRESULT { ++callbacks; return S_OK; },
                [&]() -> HRESULT { ++callbacks; return S_OK; },
                [] { return DWORD{0}; });
        };
        check(!begin() && recovery.fatal && recovery.lastFailure == D3DERR_DRIVERINTERNALERROR && callbacks == 0,
            "driver internal error is terminal without a reset attempt");
        device.cooperative = S_OK;
        check(!begin() && callbacks == 0, "terminal recovery never draws again");
        check(device_frame::RecoverableStatus(S_OK) && device_frame::RecoverableStatus(D3DERR_DEVICELOST) &&
            device_frame::RecoverableStatus(D3DERR_DEVICENOTRESET) &&
            !device_frame::RecoverableStatus(D3DERR_DRIVERINTERNALERROR) &&
            !device_frame::RecoverableStatus(E_FAIL), "recoverable cooperative statuses");
    }
    {
        DeviceModel device;
        bool lost = true;
        device_frame::Recovery recovery;
        HRESULT resetResult = E_FAIL;
        HRESULT prepareResult = S_OK;
        DWORD now = 0xFFFFF000; // window crosses the timeGetTime wrap
        const auto begin = [&] {
            return device_frame::Begin(&device, lost, true, recovery,
                [&] { return prepareResult; },
                [&] { return resetResult; },
                []() -> HRESULT { return S_OK; },
                [&] { return now; });
        };
        begin(); now += 20000; begin();
        check(!recovery.fatal && recovery.failures == 2, "two failures are not terminal");
        resetResult = D3DERR_DEVICELOST;
        for (int i = 0; i < 10; ++i) { now += 5000; begin(); }
        check(!recovery.fatal && recovery.failures == 2, "renewed loss never counts as a restore failure");
        resetResult = S_OK;
        begin();
        check(!lost && recovery.failures == 0, "successful reset clears the failure count");
        lost = true;
        device.cooperative = D3DERR_DEVICENOTRESET;
        resetResult = E_OUTOFMEMORY;
        begin(); now += 1000; begin(); now += 1000; begin(); now += 1000; begin();
        check(!recovery.fatal && recovery.failures == 4, "fast failures inside the window are not terminal");
        now += device_frame::RestoreFailureWindowMs;
        begin();
        check(recovery.fatal && recovery.lastFailure == E_OUTOFMEMORY, "persistent restore failure becomes terminal");

        device_frame::Recovery prepared;
        bool preparedLost = true;
        DWORD clock = 100;
        for (int i = 0; i < 3; ++i) {
            device_frame::Begin(&device, preparedLost, false, prepared,
                []() -> HRESULT { return E_FAIL; },
                []() -> HRESULT { return S_OK; },
                []() -> HRESULT { return S_OK; },
                [&] { return clock; });
            clock += 6000;
        }
        check(prepared.fatal && preparedLost, "persistent windowed display-mode failure is terminal");
    }

    // A18-07: a mode toggle needs both families; a missing family is never read.
    {
        struct Settings {
            bool IsWindowed = true;
            int* pWindowed_AdapterInfo = nullptr;
            int* pWindowed_DeviceInfo = nullptr;
            int* pWindowed_DeviceCombo = nullptr;
            int* pFullscreen_AdapterInfo = nullptr;
            int* pFullscreen_DeviceInfo = nullptr;
            int* pFullscreen_DeviceCombo = nullptr;
        } settings;
        int adapter = 0, deviceInfo = 0, combo = 0;
        check(!device_mode_family::Has(settings, true) && !device_mode_family::Has(settings, false),
            "no family before enumeration");
        settings.pWindowed_AdapterInfo = &adapter;
        settings.pWindowed_DeviceInfo = &deviceInfo;
        check(!device_mode_family::Has(settings, true), "incomplete family is absent");
        settings.pWindowed_DeviceCombo = &combo;
        check(device_mode_family::Has(settings, true) && !device_mode_family::Has(settings, false),
            "windowed-only enumeration has no fullscreen family");
        check(!device_mode_family::CanToggle(settings, true),
            "fullscreen toggle refused without a fullscreen combination");
        settings.pFullscreen_AdapterInfo = &adapter;
        settings.pFullscreen_DeviceInfo = &deviceInfo;
        settings.pFullscreen_DeviceCombo = &combo;
        check(device_mode_family::CanToggle(settings, true), "toggle allowed with both families");
        settings.IsWindowed = false;
        settings.pWindowed_DeviceCombo = nullptr;
        check(!device_mode_family::CanToggle(settings, false), "return to windowed refused without its family");
    }

    // A18-06: the 512-byte adapter description is classified in place.
    {
        char description[MAX_DEVICE_IDENTIFIER_STRING];
        std::memset(description, 'x', sizeof(description));
        std::memcpy(description + sizeof(description) - 9, "Intel 810", 9); // no terminator
        const std::string before(description, sizeof(description));
        const auto detected = adapter_quirks::Classify(description, sizeof(description));
        check(detected.intel && std::string(description, sizeof(description)) == before,
            "long legacy description classified without copy or mutation");
        const auto bounded = adapter_quirks::Classify(description, sizeof(description) - 1);
        check(!bounded.intel, "classification never reads past its capacity");
        const auto mx = adapter_quirks::Classify("NVIDIA GeForce4 MX 440", 64);
        const auto gtx = adapter_quirks::Classify("NVIDIA GeForce GTX 1060", 64);
        const auto radeon = adapter_quirks::Classify("AMD Radeon RX 580", 64);
        const auto other = adapter_quirks::Classify("Microsoft Basic Render Driver", 64);
        check(mx.vgaId == 0 && mx.nvidia && gtx.vgaId == 1 && radeon.vgaId == 2 && !radeon.nvidia &&
            other.vgaId == -1 && !other.intel, "legacy vendor classification is preserved");
        check(adapter_quirks::Classify(nullptr, 16).vgaId == -1, "missing description is unclassified");
    }

    // A19-06: frame-rate samples continue across the timeGetTime wrap.
    {
        frame_rate::Sampler sampler;
        float fps = -1.0f;
        DWORD now = 0xFFFFF000;
        check(!sampler.Frame(now, fps), "first frame starts the interval");
        for (int i = 0; i < 58; ++i) check(!sampler.Frame(now += 30, fps) || false, "no sample inside the interval");
        now += 2000; // crosses zero
        check(sampler.Frame(now, fps) && fps > 0.0f && fps < 60.0f, "sample published after the wrap");
        check(!sampler.Frame(now + 1000, fps), "next interval restarts after the wrap");
    }

    // A19-05: the desktop ramp is captured once and restored exactly.
    {
        D3DGAMMARAMP desktop{};
        for (int i = 0; i < 256; ++i) desktop.red[i] = static_cast<WORD>(i * 200);
        D3DGAMMARAMP display = desktop;
        desktop_gamma::Saved<D3DGAMMARAMP> saved;
        int reads = 0;
        const auto read = [&](D3DGAMMARAMP& ramp) { ++reads; ramp = display; return true; };
        const auto write = [&](const D3DGAMMARAMP& ramp) { display = ramp; return true; };
        check(saved.Restore(write) && std::memcmp(&display, &desktop, sizeof(display)) == 0,
            "nothing is restored before a change");
        check(saved.Capture(read) && reads == 1, "first change captures the user ramp");
        std::memset(&display, 0x7F, sizeof(display));
        check(saved.Capture(read) && reads == 1, "later changes keep the first capture");
        check(saved.Restore(write) && std::memcmp(&display, &desktop, sizeof(display)) == 0 && !saved.Captured(),
            "exit restores the user ramp, not a linear ramp");
        desktop_gamma::Saved<D3DGAMMARAMP> blocked;
        check(!blocked.Capture([](D3DGAMMARAMP&) { return false; }) && !blocked.Captured(),
            "failed capture blocks the change");
        desktop_gamma::Saved<D3DGAMMARAMP> retry;
        retry.Capture(read);
        check(!retry.Restore([](const D3DGAMMARAMP&) { return false; }) && retry.Captured(),
            "failed restore keeps the ramp for a later attempt");
    }

    // A19-01: playback status uses DSBSTATUS_PLAYING and never an HRESULT.
    {
        check(sound_playback::PlayingStatus == DSBSTATUS_PLAYING, "playing bit matches dsound.h");
        Buffer idle, playing, looping, lostBuffer, failed;
        playing.status = DSBSTATUS_PLAYING;
        looping.status = DSBSTATUS_PLAYING | DSBSTATUS_LOOPING;
        lostBuffer.status = DSBSTATUS_BUFFERLOST;
        failed.status = DSBSTATUS_PLAYING;
        failed.result = DSERR_INVALIDCALL;
        Buffer* none[] = {nullptr, &idle};
        Buffer* second[] = {&idle, &playing};
        Buffer* loop[] = {&looping};
        Buffer* lost[] = {&lostBuffer, &failed};
        check(!sound_playback::AnyPlaying<Buffer>(nullptr, 3), "missing buffer table is not playing");
        check(!sound_playback::AnyPlaying(none, 2), "idle and absent buffers are not playing");
        check(sound_playback::AnyPlaying(second, 2), "any playing buffer reports playback");
        check(!sound_playback::AnyPlaying(second, 1), "count bounds the scan");
        check(sound_playback::AnyPlaying(loop, 1), "looping playback reports playback");
        check(!sound_playback::AnyPlaying(lost, 2), "lost buffer or failed query is not playback");
    }

    check(sound_playback::VolumeForSound(101, DSBVOLUME_MAX) == -1800,
        "rain is quieter at maximum effects volume");
    check(sound_playback::VolumeForSound(113, -600) == -2400,
        "snow remains quieter after an effects volume change");
    check(sound_playback::VolumeForSound(101, -9000) == DSBVOLUME_MIN &&
        sound_playback::VolumeForSound(113, DSBVOLUME_MIN) == DSBVOLUME_MIN,
        "weather volume respects the DirectSound minimum and mute");
    check(sound_playback::VolumeForSound(102, -600) == -600,
        "other effects retain the configured volume");

    // A19-02: non-PCM extra bytes are allocated with the structure and must fit the chunk.
    {
        check(wave_format::AllocationBytes(1, 16, 0) == sizeof(WAVEFORMATEX), "PCM keeps the 18-byte format");
        check(!wave_format::HasExtraSize(1, 18), "PCM never reads cbSize");
        check(wave_format::HasExtraSize(2, 50) && wave_format::AllocationBytes(2, 50, 32) == 50,
            "ADPCM extra bytes are allocated after WAVEFORMATEX");
        check(wave_format::AllocationBytes(2, 50, 33) == 0 && wave_format::AllocationBytes(2, 50, 65535) == 0,
            "extra bytes beyond the fmt chunk are rejected");
        check(!wave_format::HasExtraSize(3, 16) && wave_format::AllocationBytes(3, 16, 0) == 18,
            "a 16-byte non-PCM chunk has no cbSize");
        check(wave_format::AllocationBytes(1, 15, 0) == 0, "short fmt chunk rejected");
        const std::size_t bytes = wave_format::AllocationBytes(2, 50, 32);
        std::vector<unsigned char> storage(bytes + 4, 0xCD);
        std::memset(storage.data() + sizeof(WAVEFORMATEX), 0x11, 32);
        check(storage[bytes] == 0xCD && storage[bytes + 3] == 0xCD, "extra bytes end inside the allocation");
    }

    // C-30: texture names are data, never a format; the request is not rewritten.
    {
        static Row rows[5]{};
        strcpy_s(rows[0].szFileName, "Effect\\fire.wys");
        strcpy_s(rows[2].szFileName, "UI\\%n%n%s.wys");
        strcpy_s(rows[3].szFileName, "x");
        strcpy_s(rows[4].szFileName, "UI\\m0101.wyt");
        char request[32] = "Effect\\fire.wyt";
        check(model_texture_748::Find(request, rows, 5) == 0 && std::strcmp(request, "Effect\\fire.wyt") == 0,
            "extension alias matches without rewriting the caller");
        char percent[32] = "UI\\%n%n%s.wyt";
        check(model_texture_748::Find(percent, rows, 5) == 2, "percent sequences in table rows are inert");
        char shortName[4] = "ab";
        check(model_texture_748::Find(shortName, rows, 5) == -1, "short names and empty rows never match");
        char exact[32] = "UI\\m0101.wyt";
        check(model_texture_748::Find(exact, rows, 5) == 4, "exact name matches");
    }

    // C-04 and A23-05: .msa structure limits and UV-animation preconditions.
    {
        check(msa_load::ValidHeader(274, 32, 1) && msa_load::ValidHeader(322, 24, 32), "shipped layouts accepted");
        check(!msa_load::ValidHeader(274, 32, 0) && !msa_load::ValidHeader(274, 32, 33),
            "attribute count limited to the 32-entry arrays");
        check(!msa_load::ValidHeader(322, 0, 1) && !msa_load::ValidHeader(322, 13, 1) &&
            !msa_load::ValidHeader(322, 8, 1) && !msa_load::ValidHeader(322, 4096, 1), "invalid stride rejected");
        check(!msa_load::ValidHeader(274, 36, 1), "UV-animated layout must be RDVERTEX2");
        msa_load::VertexPlan plan;
        check(msa_load::PlanVertices(274, 32, 32 * 4, 128, plan) && plan.vertices == 4 &&
            plan.bufferBytes == 160 && plan.readBytes == 128 && plan.loadedStride == 40 && plan.addUv,
            "added UV set sized per vertex");
        check(msa_load::PlanVertices(322, 24, 48, 48, plan) && !plan.addUv && plan.bufferBytes == 48,
            "color mesh loaded as stored");
        check(!msa_load::PlanVertices(274, 32, 32, 32, plan) && plan.vertices == 0, "single vertex rejected");
        check(!msa_load::PlanVertices(274, 32, 32 * 4, 127, plan), "truncated vertex payload rejected");
        check(!msa_load::PlanVertices(274, 4, 0xFFFFFFFC, 0xFFFFFFFFFull, plan), "buffer size overflow rejected");
        check(!msa_load::PlanVertices(274, 0, 64, 64, plan), "zero stride rejected without division");
        msa_load::Range ranges[2]{{0, 0, 2, 0, 4}, {1, 2, 1, 2, 2}};
        check(msa_load::RangesFit(ranges, 2, 4, 9), "ranges at the buffer end accepted");
        check(!msa_load::RangesFit(ranges, 2, 3, 9), "vertex range beyond buffer rejected");
        check(!msa_load::RangesFit(ranges, 2, 4, 8), "face range beyond index buffer rejected");
        msa_load::Range wrap{0, 0xFFFFFFFF, 1, 0xFFFFFFFF, 2};
        check(!msa_load::RangesFit(&wrap, 1, 4, 9), "range arithmetic cannot wrap");
        check(!msa_load::RangesFit(ranges, 33, 4, 9) && !msa_load::RangesFit(nullptr, 1, 4, 9),
            "range table bounded");
        char stem[8]{};
        check(msa_load::CopyStem(stem, sizeof(stem), "abc.wyt", 8) && std::strcmp(stem, "abc") == 0, "stem stops at dot");
        check(!msa_load::CopyStem(stem, sizeof(stem), "abcdefghij", 11) && stem[0] == 0, "long stem rejected");
        const char unterminated[3] = {'a', 'b', 'c'};
        check(msa_load::CopyStem(stem, sizeof(stem), unterminated, 3) && std::strcmp(stem, "abc") == 0,
            "unterminated texture field bounded");
        check(msa_load::UvAnimatable(true, 530, 40, 3), "loaded RDVERTEX2 mesh is animatable");
        check(!msa_load::UvAnimatable(false, 530, 40, 3), "failed load without buffer is not animatable");
        check(!msa_load::UvAnimatable(true, 274, 32, 3) && !msa_load::UvAnimatable(true, 530, 32, 3),
            "other layouts are not rewritten");
    }

    // A23-07: shader palette budget and declaration stride.
    {
        skin_mesh_load::Header header{0, 1, 0, 32 + 4 * 2, 2, 27, 3, 3};
        check(skin_mesh_load::ShaderPaletteBudget == 27, "c9..c91 leave room for 27 bones before c92");
        check(skin_mesh_load::ShaderCompatible(header), "27-bone weighted mesh accepted");
        auto over = header; over.palette = 28;
        check(!skin_mesh_load::ShaderCompatible(over), "bone 28 would overlap the inverse view constants");
        auto stride = header; stride.stride = 36;
        check(!skin_mesh_load::ShaderCompatible(stride), "stride must match the influence declaration");
        auto rigid = header; rigid.palette = 0; rigid.influences = 0; rigid.stride = 24;
        check(skin_mesh_load::ShaderCompatible(rigid), "rigid part keeps its existing handling");
        rigid.influences = 1;
        check(!skin_mesh_load::ShaderCompatible(rigid), "weighted part without bones rejected");
    }

    // A23-04: animation indices and the skeleton-49 tick trim.
    {
        check(skin_animation::ValidSkeleton(0, 100) && skin_animation::ValidSkeleton(99, 100) &&
            !skin_animation::ValidSkeleton(100, 100) && !skin_animation::ValidSkeleton(-1, 100),
            "skeleton index excludes the capacity");
        check(skin_animation::PlayableTicks(49, 0) == 0 && skin_animation::PlayableTicks(49, 1) == 0 &&
            skin_animation::PlayableTicks(49, 2) == 0 && skin_animation::PlayableTicks(49, 5) == 3,
            "skeleton 49 short cuts never wrap");
        check(skin_animation::PlayableTicks(10, 1) == 1, "other skeletons keep every tick");
        check(skin_animation::ValidCut(4, 5, 186) && !skin_animation::ValidCut(5, 5, 186) &&
            !skin_animation::ValidCut(-1, 5, 186) && !skin_animation::ValidCut(186, 200, 186),
            "cut index bounded by loaded cuts and table capacity");
    }

    // A23-06: one restore pass advances the part cycle; each restore restarts it.
    {
        const auto& costume = fallback_costume::kCostumes[0];
        const auto pass = [&](int& part) {
            std::string meshes;
            char texture[64]{};
            char mesh[64]{};
            for (int i = 0; i < 8; ++i) {
                fallback_costume::Apply(costume, part, texture, mesh);
                meshes += mesh;
                meshes += ';';
            }
            return meshes;
        };
        int part = 1;
        const std::string first = pass(part);
        check(part != 1, "a full restore leaves the cycle away from part 1");
        int restarted = 1;
        check(pass(restarted) == first, "restarting the cycle reproduces the first pairing");
        check(pass(part) != first, "continuing the cycle re-pairs parts (defect trigger)");
    }

    // A23-08: failure paths restore the skinning pipeline exactly once.
    {
        int restores = 0;
        const auto draw = [&](bool fail) {
            scope_restore::Guard guard([&] { ++restores; });
            if (fail) return false;
            ++restores; // normal path restores inline
            guard.Dismiss();
            return true;
        };
        check(!draw(true) && restores == 1, "early return restores state");
        check(draw(false) && restores == 2, "normal path restores once");
    }

    // Whole shipped corpus satisfies the new loader limits.
    {
        const auto runtime = FindRuntime();
        check(!runtime.empty(), "shipped client runtime located");
        if (!runtime.empty()) {
            std::ifstream list(runtime / "Mesh" / "MeshList.txt");
            std::string line;
            int checked = 0;
            int rejected = 0;
            while (std::getline(list, line)) {
                std::istringstream fields(line);
                int index = 0;
                std::string name;
                if (!(fields >> index >> name)) continue;
                std::string relative;
                for (std::size_t i = 0; i < name.size(); ++i) {
                    if (name[i] == '\\') {
                        relative += '/';
                        while (i + 1 < name.size() && name[i + 1] == '\\') ++i;
                    } else {
                        relative += name[i];
                    }
                }
                const auto path = runtime / relative;
                if (!std::filesystem::is_regular_file(path)) continue; // missing entries stay retained-empty
                ++checked;
                if (!ShippedMsaPasses(ReadAll(path))) {
                    ++rejected;
                    std::fprintf(stderr, "rejected shipped mesh: %s\n", relative.c_str());
                }
            }
            check(checked > 1800 && rejected == 0, "every shipped .msa passes the hardened loader limits");

            int skins = 0;
            int skinRejected = 0;
            for (const auto& entry : std::filesystem::directory_iterator(runtime / "mesh")) {
                const auto extension = entry.path().extension().string();
                if (extension != ".msh" && extension != ".MSH") continue;
                const auto data = ReadAll(entry.path());
                skin_mesh_load::Header header{};
                skin_mesh_load::Payload payload;
                ++skins;
                if (data.size() < sizeof(header)) { ++skinRejected; continue; }
                std::memcpy(&header, data.data(), sizeof(header));
                if (!skin_mesh_load::Plan(header, data.size(), 40, payload) ||
                    !skin_mesh_load::ShaderCompatible(header)) {
                    ++skinRejected;
                    std::fprintf(stderr, "rejected shipped skin: %s\n", entry.path().filename().string().c_str());
                }
            }
            check(skins > 1500 && skinRejected == 0, "every shipped .msh passes the shader limits");
        }
    }
    return failures;
}
