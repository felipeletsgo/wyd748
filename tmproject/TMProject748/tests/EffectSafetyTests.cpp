#include "../internal/render/effects/core/EffectGuards.h"
#include "../internal/render/effects/core/EffectRenderState.h"
#include "../internal/render/effects/core/EffectTiming.h"
#include "SourceMethod.h"

#include <windows.h>

#include <algorithm>
#include <climits>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iterator>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace
{
struct FakeVertex
{
    float position[3];
    unsigned int diffuse;
    float uv[2];
};
static_assert(sizeof(FakeVertex) == 24, "effect vertices are 24 bytes (FVF 322)");

struct FakeDesc
{
    unsigned int Size = 0;
    unsigned int FVF = 0;
};

// Reports two vertices and keeps two guard vertices behind them.
struct FakeBuffer
{
    FakeVertex storage[4]{
        {{1, 2, 3}, 7, {4, 5}}, {{6, 7, 8}, 9, {10, 11}},
        {{12, 13, 14}, 15, {16, 17}}, {{18, 19, 20}, 21, {22, 23}}};
    FakeDesc desc{2 * sizeof(FakeVertex), 322};
    long getResult = 0;
    long lockResult = 0;
    long unlockResult = 0;
    bool nullData = false;
    int gets = 0;
    int locks = 0;
    int unlocks = 0;

    long GetDesc(FakeDesc* out)
    {
        ++gets;
        *out = desc;
        return getResult;
    }
    long Lock(unsigned int, unsigned int, void** out, unsigned int)
    {
        ++locks;
        *out = nullData ? nullptr : storage;
        return lockResult;
    }
    long Unlock()
    {
        ++unlocks;
        return unlockResult;
    }
};

bool Fill(FakeBuffer* buffer, unsigned int color)
{
    return effect_vertex_lock::TryFillDiffuse<FakeVertex, FakeDesc>(buffer, color);
}

struct FakeMesh
{
    int m_nTextureIndex[32]{};
};

struct StateDevice
{
    std::map<int, unsigned long> render;
    std::map<int, unsigned long> stage0;
    void SetRenderState(D3DRENDERSTATETYPE state, DWORD value) { render[state] = value; }
    void SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value)
    {
        if (stage == 0)
            stage0[type] = value;
    }
};

std::filesystem::path ClientRoot()
{
    const auto marker = std::filesystem::path("internal") / "render" / "effects";
    std::error_code error;
    auto current = std::filesystem::current_path(error);
    for (int depth = 0; depth < 10 && !current.empty(); ++depth) {
        if (std::filesystem::exists(current / marker, error))
            return current;
        const auto nested = current / "tmproject" / "TMProject748";
        if (std::filesystem::exists(nested / marker, error))
            return nested;
        const auto parent = current.parent_path();
        if (parent == current)
            break;
        current = parent;
    }
    wchar_t executable[MAX_PATH]{};
    const auto length = GetModuleFileNameW(nullptr, executable, MAX_PATH);
    if (!length || length == MAX_PATH)
        return {};
    current = std::filesystem::path(executable).parent_path();
    for (int depth = 0; depth < 10 && !current.empty(); ++depth) {
        const auto nested = current / "tmproject" / "TMProject748";
        if (std::filesystem::exists(nested / marker, error))
            return nested;
        const auto parent = current.parent_path();
        if (parent == current)
            break;
        current = parent;
    }
    return {};
}

std::string ReadFile(const std::filesystem::path& path)
{
    std::ifstream input(path, std::ios::binary);
    if (!input)
        return {};
    std::string text{std::istreambuf_iterator<char>(input), {}};
    text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
    return text;
}

std::string ReadSource(const char* relative)
{
    const auto root = ClientRoot();
    return root.empty() ? std::string{} : ReadFile(root / relative);
}

std::string Squeeze(const std::string& body)
{
    auto code = source_contract::MaskCpp(body);
    code.erase(std::remove_if(code.begin(), code.end(), [](unsigned char c) {
        return c == ' ' || c == '\t' || c == '\n';
    }), code.end());
    return code;
}

// Files owned by the device-reset lane; their contracts are checked there.
bool OwnedElsewhere(const std::filesystem::path& file)
{
    const auto name = file.filename().string();
    return name.rfind("TMEffectSkinMesh.", 0) == 0 || name.rfind("TMEffectDust.", 0) == 0 ||
        name.rfind("TMDust.", 0) == 0;
}

void TestVertexLock(int& checks, int& failures)
{
    auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) { ++failures; std::fprintf(stderr, "FAIL EffectSafety vertex lock: %s\n", name); }
    };
    check(!Fill(nullptr, 0xFF00FF00u), "a mesh whose load failed (null VB) is skipped");

    const FakeBuffer original;
    FakeBuffer ok;
    check(Fill(&ok, 0xFF00FF00u), "a valid buffer accepts the color");
    check(ok.gets == 1 && ok.locks == 1 && ok.unlocks == 1, "one lock is balanced by one unlock");
    check(ok.storage[0].diffuse == 0xFF00FF00u && ok.storage[1].diffuse == 0xFF00FF00u,
        "every reported vertex receives the color");
    check(std::memcmp(ok.storage[0].position, original.storage[0].position, sizeof(float) * 3) == 0 &&
        std::memcmp(ok.storage[1].uv, original.storage[1].uv, sizeof(float) * 2) == 0,
        "positions and texture coordinates are kept");
    check(std::memcmp(&ok.storage[2], &original.storage[2], 2 * sizeof(FakeVertex)) == 0,
        "the write stops at the reported buffer size");

    FakeBuffer partial;
    partial.desc.Size = 2 * sizeof(FakeVertex) + 10;
    check(Fill(&partial, 1u) && partial.storage[1].diffuse == 1u &&
        std::memcmp(&partial.storage[2], &original.storage[2], 2 * sizeof(FakeVertex)) == 0,
        "a size that is not a whole vertex count never writes past the last whole vertex");

    for (int fault = 0; fault < 4; ++fault) {
        FakeBuffer buffer;
        switch (fault) {
        case 0: buffer.getResult = E_FAIL; break;
        case 1: buffer.lockResult = D3DERR_INVALIDCALL; break;
        case 2: buffer.nullData = true; break;
        case 3: buffer.unlockResult = E_FAIL; break;
        }
        check(!Fill(&buffer, 0xFFFFFFFFu), "a device failure is reported");
        check(buffer.locks == (fault == 0 ? 0 : 1), "a failed GetDesc never locks");
        check(buffer.unlocks == (fault >= 2 ? 1 : 0), "only a successful lock is unlocked");
        if (fault != 3)
            check(std::memcmp(buffer.storage, original.storage, sizeof(buffer.storage)) == 0,
                "a failed lock leaves memory unchanged (no write through an unset pointer)");
    }

    int writes = 0;
    FakeBuffer custom;
    check(effect_vertex_lock::TryWrite<FakeVertex, FakeDesc>(&custom,
        [&](FakeVertex* vertices, unsigned int count) {
            writes += static_cast<int>(count);
            vertices[0].uv[0] += 1.0f;
        }) && writes == 2 && custom.storage[0].uv[0] == 5.0f,
        "the general writer receives the vertices and their count once");
}

void TestSwingEmitters(int& checks, int& failures)
{
    auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) { ++failures; std::fprintf(stderr, "FAIL EffectSafety swing emitters: %s\n", name); }
    };
    using swing_emitter::Family;
    const Family families[] = {swing_emitter::Berserk, swing_emitter::HandFour, swing_emitter::Assert,
        swing_emitter::MagicWeapon, swing_emitter::ArmorClass};

    // Frame periods of about 30, 60, 144 and 500 FPS. All buffs are active
    // together, and the window crosses the 2^31 tick boundary.
    for (unsigned int period : {33u, 16u, 7u, 2u}) {
        swing_emitter::Clock clock;
        int counts[swing_emitter::FamilyCount]{};
        const unsigned int start = 0x7FFFFE00u;
        for (unsigned int t = 0; t < 1000; t += period)
            for (const auto family : families)
                if (clock.TryEmit(family, start + t))
                    ++counts[family];
        for (const auto family : families) {
            const unsigned int interval = swing_emitter::kIntervalMs[family];
            const int lowest = static_cast<int>(1000 / (interval + period));
            const int highest = static_cast<int>(1000 / interval) + 1;
            check(counts[family] >= lowest && counts[family] <= highest,
                "each family emits at its own interval, independent of the frame rate and of other buffs");
        }
        check(counts[swing_emitter::Berserk] <= 10,
            "Berserk emits at most ten billboards per swing per second (was one per frame)");
        check(counts[swing_emitter::MagicWeapon] > 0 && counts[swing_emitter::ArmorClass] > 0,
            "Assert no longer suppresses Magic Weapon and Armor Class");
    }

    swing_emitter::Clock clock;
    check(clock.TryEmit(swing_emitter::Assert, 1000) && clock.TryEmit(swing_emitter::MagicWeapon, 1000),
        "one family emitting does not consume another family's interval");
    check(!clock.TryEmit(swing_emitter::Assert, 1050) && clock.TryEmit(swing_emitter::Assert, 1051),
        "an emission needs strictly more than the interval and advances its own clock");
    check(!clock.TryEmit(static_cast<Family>(swing_emitter::FamilyCount), 5000) &&
        !clock.TryEmit(static_cast<Family>(-1), 5000), "an unknown family never emits");

    swing_emitter::Clock high;
    check(high.TryEmit(swing_emitter::Berserk, 0x80000100u),
        "the first emission happens when the server tick has its high bit set");
}

void TestTiming(int& checks, int& failures)
{
    auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) { ++failures; std::fprintf(stderr, "FAIL EffectSafety timing: %s\n", name); }
    };
    using namespace effect_timing;

    // A22-06: Town Portal starts with m_dwLastTime = 0.
    check(IntervalElapsed(0x80000100u, 0, 100), "Town Portal emits after 24.8 days of uptime");
    check(!IntervalElapsed(1100, 1000, 100) && IntervalElapsed(1101, 1000, 100), "interval boundary is exclusive");
    check(!IntervalElapsed(0x10u, 0xFFFFFFF0u, 100) && IntervalElapsed(0x100u, 0xFFFFFF00u, 100),
        "the interval gate is correct across the 2^32 wrap");

    // A22-06: Freeze Blade segments may start up to 500 ms in the future.
    check(!HasStarted(0x7FFFFF00u, 0x7FFFFF00u + 100), "a future start is not reached at high ticks");
    check(HasStarted(0x7FFFFF00u + 100, 0x7FFFFF00u + 100), "the start tick counts as started");
    check(HasStarted(0x80000100u, 0x7FFFFF00u) && HasStarted(0x100u, 0xFFFFFF00u),
        "a start before the 2^31 or 2^32 boundary counts as started");

    // A21-11: TMFireEffect deletion across the wrap and after a clock step back.
    check(LifetimeExceeded(0x900u, 0xFFFFFF00u, 2500), "an effect created before the wrap expires after it");
    check(!LifetimeExceeded(0xFFFFFF00u - 5, 0xFFFFFF00u, 2500), "a clock that stepped back does not expire it");
    check(!LifetimeExceeded(1000 + 2500, 1000, 2500) && LifetimeExceeded(1000 + 2501, 1000, 2500),
        "the lifetime boundary matches the previous strict comparison");
    check(!LifetimeExceeded(0xFFFFFFFFu, 0, 0), "a zero lifetime never expires");

    // A21-11: billboard scale velocity after long uptime.
    check(ElapsedFloat(0x7FFF0005u, 0x7FFF0000u) == 5.0f && ElapsedFloat(0x100u, 0xFFFFFF00u) == 512.0f,
        "elapsed time keeps millisecond precision at high ticks and across the wrap");

    // A21-11: TMEffectMesh cycle.
    check(CycleProgress(1234, 0, 0) == 0.0f, "a zero cycle gives progress 0 instead of a division by zero");
    check(CycleProgress(1500, 0, 1000) == 0.5f && CycleProgress(0x100u, 0xFFFFFF00u, 1000) == 0.512f,
        "cycle progress uses the tick difference");

    // A21-11: TMEffectMeshRotate countdown phase. The start is staggered up to
    // 900 ms into the future; the phase must not jump when time passes it.
    for (unsigned int period : {1000u, 2000u}) {
        const unsigned int start = 0xFFFFFF00u;
        bool equalBefore = true;
        bool continuous = true;
        for (unsigned int step = 0; step < 4000; ++step) {
            const unsigned int now = start - 900 + step;
            const unsigned int phase = CountdownPhase(start, now, period);
            if (static_cast<int>(start - now) >= 0 && phase != (start - now) % period)
                equalBefore = false;
            const unsigned int next = CountdownPhase(start, now + 1, period);
            if ((phase + period - next) % period != 1)
                continuous = false;
        }
        check(equalBefore, "the phase equals the previous expression until the start tick");
        check(continuous, "the phase advances by one millisecond per millisecond, without a jump");
    }
    check(CountdownPhase(5, 7, 0) == 0, "a zero period gives phase 0");

    // A21-11: TMEffectBillBoard3::SetShort window.
    check(ShortWindowProgress(1450, 1400, 1000, 500) == 0.5f, "a valid short window keeps its progress");
    check(ShortWindowProgress(1500, 1500, 1000, 500) == 1.0f && ShortWindowProgress(1700, 1600, 1000, 500) == 1.0f,
        "a short time at or after the expiry gives a complete window instead of a zero or wrapped divisor");
}

void TestBounds(int& checks, int& failures)
{
    auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) { ++failures; std::fprintf(stderr, "FAIL EffectSafety bounds: %s\n", name); }
    };
    // A21-08: TMEffectSpark.
    check(effect_spark::ClampCount(-3) == 0 && effect_spark::ClampCount(0) == 0 &&
        effect_spark::ClampCount(3) == 3 && effect_spark::ClampCount(5) == 5 &&
        effect_spark::ClampCount(8) == 5 && effect_spark::ClampCount(INT_MAX) == 5,
        "the spark count is clamped to the five child slots");
    check(effect_spark::ParentLifetime(1000) == 900, "current callers keep the 100 ms margin");
    check(effect_spark::ParentLifetime(100) == 100 && effect_spark::ParentLifetime(50) == 50 &&
        effect_spark::ParentLifetime(1) == 1 && effect_spark::ParentLifetime(0) == 0,
        "short lifetimes neither wrap nor become infinite");
    bool ordered = true;
    for (unsigned int child = 1; child <= 1500; ++child) {
        const unsigned int parent = effect_spark::ParentLifetime(child);
        if (parent == 0 || parent > child)
            ordered = false;
        for (unsigned int delay : {0u, 1u, 7u}) {
            const unsigned int parentCreate = 0xFFFFFF00u;
            const unsigned int childCreate = parentCreate + delay;
            for (unsigned int t = 0; t <= child + delay + 3; t += 1) {
                const unsigned int now = parentCreate + t;
                if (effect_timing::LifetimeExceeded(now, childCreate, child) &&
                    !effect_timing::LifetimeExceeded(now, parentCreate, parent))
                    ordered = false;
            }
        }
    }
    check(ordered, "the parent never outlives a child it moves through a raw pointer");

    // A22-04: Thunder Bolt (three rows) and Haste (five colors).
    check(effect_bounds::RowOr(5, 3, 0) == 0 && effect_bounds::RowOr(3, 3, 0) == 0 &&
        effect_bounds::RowOr(-1, 3, 0) == 0, "an out-of-table variant selects the fallback row");
    check(effect_bounds::RowOr(0, 3, 0) == 0 && effect_bounds::RowOr(2, 3, 0) == 2 &&
        effect_bounds::RowOr(4, 5, 0) == 4, "valid variants keep their own row");
}

void TestShadeMask(int& checks, int& failures)
{
    auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) { ++failures; std::fprintf(stderr, "FAIL EffectSafety shade mask: %s\n", name); }
    };
    static char mask[shade_mask::kSize][shade_mask::kSize];
    std::memset(mask, 5, sizeof(mask));
    int height = -1;
    for (const auto& outside : std::initializer_list<std::pair<int, int>>{{-1, 10}, {128, 10}, {10, -1}, {10, 128}, {-129, 0}})
        check(!shade_mask::Sample(mask, outside.first, outside.second, height),
            "an owner sample outside this block is not read");
    check(height == -1, "a rejected sample leaves the height unchanged");
    check(!shade_mask::Sample(nullptr, 1, 1, height), "a missing mask is rejected");

    mask[60][60] = 10;
    check(shade_mask::Sample(mask, 60, 60, height) && height == 10, "a low cell keeps its own height");
    mask[10][10] = 20;
    check(shade_mask::Sample(mask, 10, 10, height) && height == 5, "a high cell takes the +y neighbor first");
    mask[20][20] = 20;
    mask[21][20] = 15;
    mask[19][20] = 3;
    check(shade_mask::Sample(mask, 20, 20, height) && height == 3, "then the -y neighbor");
    mask[30][30] = 20;
    mask[31][30] = 15;
    mask[29][30] = 15;
    mask[30][31] = 15;
    mask[30][29] = 15;
    check(shade_mask::Sample(mask, 30, 30, height) && height == 20, "a cell without a low neighbor keeps its height");
    mask[127][40] = 20;
    mask[126][40] = 4;
    check(shade_mask::Sample(mask, 40, 127, height) && height == 4,
        "at the last row the +y neighbor is skipped, not read past the map");
    mask[0][0] = 20;
    mask[1][0] = 15;
    mask[0][1] = 7;
    check(shade_mask::Sample(mask, 0, 0, height) && height == 7,
        "at the first row and column the -y and -x neighbors are skipped");
}

void TestMeshAndDeviceState(int& checks, int& failures)
{
    auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) { ++failures; std::fprintf(stderr, "FAIL EffectSafety mesh state: %s\n", name); }
    };
    FakeMesh mesh;
    mesh.m_nTextureIndex[0] = 91;
    {
        effect_mesh_state::TextureOverride<FakeMesh> texture(&mesh, true, -1);
        check(mesh.m_nTextureIndex[0] == -1, "the draw uses the instance texture");
    }
    check(mesh.m_nTextureIndex[0] == 91, "the cached mesh keeps its own texture after the draw");
    {
        effect_mesh_state::TextureOverride<FakeMesh> texture(&mesh, false, 33);
        check(mesh.m_nTextureIndex[0] == 91, "types without an instance texture leave the mesh alone");
    }
    {
        effect_mesh_state::TextureOverride<FakeMesh> texture(nullptr, true, 33);
    }
    check(mesh.m_nTextureIndex[0] == 91, "a missing mesh is ignored");

    const std::map<int, unsigned long> defaults{{D3DRS_CULLMODE, 3}, {D3DRS_LIGHTING, 1}, {D3DRS_SRCBLEND, 2},
        {D3DRS_ALPHAFUNC, 7}, {D3DRS_ALPHATESTENABLE, 1}, {D3DRS_ZWRITEENABLE, 1}, {D3DRS_DESTBLEND, 6}};
    for (bool bright : {true, false}) {
        StateDevice device;
        device.render = defaults;
        device.stage0[D3DTSS_ALPHAOP] = 2;
        // The states TMEffectMeshRotate::Render sets before drawing.
        device.SetRenderState(D3DRS_ALPHATESTENABLE, 0);
        device.SetRenderState(D3DRS_ZWRITEENABLE, 0);
        device.SetRenderState(D3DRS_LIGHTING, 0);
        device.SetRenderState(D3DRS_ALPHAFUNC, 8);
        device.SetRenderState(D3DRS_SRCBLEND, 5);
        if (bright) {
            device.SetRenderState(D3DRS_CULLMODE, 1);
            device.SetRenderState(D3DRS_DESTBLEND, 2);
        } else {
            device.SetRenderState(D3DRS_DESTBLEND, 6);
            device.SetTextureStageState(0, D3DTSS_ALPHAOP, 4);
            device.SetRenderState(D3DRS_LIGHTING, 1);
        }
        effect_render_state::RestoreDefaults(device, !bright);
        bool restored = true;
        for (const auto& [state, value] : defaults)
            if (device.render[state] != value)
                restored = false;
        check(restored, "every render state an effect changed returns to its default");
        check(device.stage0[D3DTSS_ALPHAOP] == 2, "texture stage 0 alpha returns to SELECTARG1");
    }
}

// Every scalar and pointer member of these classes has a default
// initializer, so a variant that skips an assignment cannot read garbage.
void TestMemberDefaults(int& checks, int& failures)
{
    auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) { ++failures; std::fprintf(stderr, "FAIL EffectSafety member defaults: %s\n", name); }
    };
    const std::regex scalar(
        R"(^\s*(unsigned\s+int|unsigned\s+char|int|float|char|DWORD|bool|short|[A-Za-z_]\w*\s*\*+)\s*m_\w+\s*(\[[^\]]*\])?\s*(=[^;]*|\{[^;]*\})?\s*;\s*$)");
    for (const char* header : {
             "internal/render/effects/skills/TMSkillMagicArrow.h",
             "internal/render/effects/skills/TMSkillFire.h",
             "internal/render/effects/skills/TMSkillSpChange.h",
             "internal/render/effects/skills/TMSkillMeteorStorm.h",
             "internal/render/effects/skills/TMFireEffect.h",
             "internal/render/effects/core/TMEffectSWSwing.h",
             "internal/render/effects/core/TMEffectSpark.h"}) {
        const auto text = source_contract::MaskCpp(ReadSource(header));
        std::istringstream lines(text);
        std::string line;
        int members = 0;
        int missing = 0;
        while (std::getline(lines, line)) {
            std::smatch match;
            if (!std::regex_match(line, match, scalar))
                continue;
            ++members;
            if (!match[3].matched)
                ++missing;
        }
        if (members < 5 || missing != 0)
            std::fprintf(stderr, "  %s: %d scalar/pointer members, %d without initializer\n", header, members, missing);
        check(members >= 5 && missing == 0, "scalar and pointer members have default initializers");
    }
}

void TestSourceContracts(int& checks, int& failures)
{
    auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) { ++failures; std::fprintf(stderr, "FAIL EffectSafety contract: %s\n", name); }
    };
    const auto root = ClientRoot();
    check(!root.empty(), "the client source tree is found");
    if (root.empty())
        return;

    // C-41 / A21-04 / A22-03: every vertex-buffer lock goes through the guard,
    // and destructors (C-34) create no effects or sounds.
    const std::regex destructor(R"((\w+)::~(\w+)\(\))");
    int sources = 0;
    int destructors = 0;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(root / "internal" / "render" / "effects")) {
        if (!entry.is_regular_file() || OwnedElsewhere(entry.path()))
            continue;
        const auto extension = entry.path().extension().string();
        if (extension != ".cpp" && extension != ".h")
            continue;
        const auto source = ReadFile(entry.path());
        const auto code = source_contract::MaskCpp(source);
        if (entry.path().filename() != "EffectGuards.h") {
            if (code.find("->Lock(") != std::string::npos || code.find(".Lock(") != std::string::npos)
                std::fprintf(stderr, "  raw lock in %s\n", entry.path().filename().string().c_str());
            check(code.find("->Lock(") == std::string::npos && code.find(".Lock(") == std::string::npos,
                "effects lock vertex buffers only through effect_vertex_lock or TrySetDiffuse");
        }
        if (extension != ".cpp")
            continue;
        ++sources;
        for (std::sregex_iterator it(code.begin(), code.end(), destructor), end; it != end; ++it) {
            if ((*it)[1] != (*it)[2])
                continue;
            ++destructors;
            const auto body = source_contract::MaskCpp(source_contract::Method(source, it->str()));
            const bool clean = !body.empty() && body.find("new ") == std::string::npos &&
                body.find("AddChild(") == std::string::npos && body.find("Play(") == std::string::npos;
            if (!clean)
                std::fprintf(stderr, "  destructor %s allocates or plays\n", it->str().c_str());
            check(clean, "effect destructors create no effects or sounds");
        }
    }
    // 39 lane sources today (42 under core/ and skills/ minus the three
    // device-lane files); each defines a destructor.
    if (sources < 35 || destructors < 35)
        std::fprintf(stderr, "  census: %d sources, %d destructors\n", sources, destructors);
    check(sources >= 35 && destructors >= 35, "the effect census found the lane sources");

    for (const auto& [file, owner] : std::initializer_list<std::pair<const char*, const char*>>{
             {"internal/render/effects/skills/TMSkillMagicArrow.cpp", "TMSkillMagicArrow"},
             {"internal/render/effects/skills/TMSkillMeteorStorm.cpp", "TMSkillMeteorStorm"},
             {"internal/render/effects/skills/TMSkillIceSpear.cpp", "TMSkillIceSpear"},
             {"internal/render/effects/skills/TMSkillBash.cpp", "TMSkillBash"}}) {
        const auto source = ReadSource(file);
        const auto frame = Squeeze(source_contract::Method(source,
            std::string("int ") + owner + "::FrameMove(unsigned int dwServerTime)"));
        const auto impact = Squeeze(source_contract::Method(source, std::string("void ") + owner + "::SpawnImpact()"));
        check(!impact.empty() && impact.find("AddChild(") != std::string::npos,
            "the impact moved to a SpawnImpact member");
        check(frame.find("if(!m_cDeleted)SpawnImpact();g_pObjectManager->DeleteObject(this);") != std::string::npos,
            "the impact is created once, at natural expiry, before the effect deletes itself");
    }

    // A21-05 / A22-03: per-instance color of shared meshes is written in
    // Render, just before the instance draws, never in FrameMove.
    struct SharedMeshWriter { const char* file; const char* owner; bool scaleInRender; };
    for (const auto& writer : std::initializer_list<SharedMeshWriter>{
             {"internal/render/effects/core/TMEffectCharge.cpp", "TMEffectCharge", true},
             {"internal/render/effects/core/TMEffectStart.cpp", "TMEffectStart", true},
             {"internal/render/effects/skills/TMSkillTownPortal.cpp", "TMSkillTownPortal", true},
             {"internal/render/effects/skills/TMSkillMagicShield.cpp", "TMSkillMagicShield", true},
             {"internal/render/effects/skills/TMSkillDoubleSwing.cpp", "TMSkillDoubleSwing", true},
             {"internal/render/effects/skills/TMSkillFreezeBlade.cpp", "TMSkillFreezeBlade", false}}) {
        const auto source = ReadSource(writer.file);
        const auto frame = Squeeze(source_contract::Method(source,
            std::string("int ") + writer.owner + "::FrameMove(unsigned int dwServerTime)"));
        const auto render = Squeeze(source_contract::Method(source, std::string("int ") + writer.owner + "::Render()"));
        check(!frame.empty() && frame.find("Diffuse<") == std::string::npos &&
            frame.find("m_pVB") == std::string::npos, "FrameMove does not write shared vertex colors");
        const auto paint = (std::min)(render.find("TryFillDiffuse<"), render.find("TrySetDiffuse<"));
        check(source_contract::InOrder(paint, render.find("->Render(")),
            "Render applies the instance color before its draw");
        if (writer.scaleInRender)
            check(frame.find("pMesh->m_fScale") == std::string::npos &&
                frame.find("pMesh->m_nTextureIndex") == std::string::npos,
                "FrameMove leaves the shared mesh scale and texture to Render");
    }

    // A21-10: TMEffectMeshRotate restores device state on every exit after
    // it changed state.
    const auto rotate = Squeeze(source_contract::Method(ReadSource("internal/render/effects/core/TMEffectMeshRotate.cpp"),
        "int TMEffectMeshRotate::Render()"));
    std::size_t restores = 0;
    for (std::size_t at = 0; (at = rotate.find("effect_render_state::RestoreDefaults(", at)) != std::string::npos; ++at)
        ++restores;
    check(restores == 3, "both missing-mesh exits and the draw exit restore the defaults");
    check(source_contract::InOrder(rotate.rfind("pMesh->Render("), rotate.rfind("RestoreDefaults(*g_pDevice,!bBright);")),
        "the draw exit restores after the last draw");

    // A21-02: the swing emitters no longer share one clock.
    const auto swing = Squeeze(ReadSource("internal/render/effects/core/TMEffectSWSwing.cpp"));
    check(swing.find("m_dwOldTime") == std::string::npos, "no shared emitter timestamp remains");
    for (const char* gate : {"m_emitters.TryEmit(swing_emitter::Berserk,dwServerTime)",
             "m_emitters.TryEmit(swing_emitter::HandFour,dwServerTime)",
             "m_emitters.TryEmit(swing_emitter::Assert,dwServerTime)",
             "m_emitters.TryEmit(swing_emitter::MagicWeapon,dwServerTime)",
             "m_emitters.TryEmit(swing_emitter::ArmorClass,dwServerTime)"})
        check(swing.find(gate) != std::string::npos, "each weapon emitter family uses its own clock");
}
}

int RunEffectSafetyTests(int& checks)
{
    int failures = 0;
    TestVertexLock(checks, failures);
    TestSwingEmitters(checks, failures);
    TestTiming(checks, failures);
    TestBounds(checks, failures);
    TestShadeMask(checks, failures);
    TestMeshAndDeviceState(checks, failures);
    TestMemberDefaults(checks, failures);
    TestSourceContracts(checks, failures);
    return failures;
}
