#include "../internal/platform/windows/RenderStateCache.h"
#include "../internal/platform/windows/NativeInventoryLighting.h"
#include <cstdio>
#include <cstring>
#include <initializer_list>

namespace {
struct Texture {};
struct Device {
    int calls = 0;
    int failAt = 0;
    HRESULT status = S_OK;
    DWORD stage = 0;
    DWORD state = 0;
    DWORD value = 0;
    Texture* texture = nullptr;
    D3DMATRIX world{};
    HRESULT Result() { return ++calls == failAt ? D3DERR_DEVICELOST : status; }
    HRESULT SetRenderState(D3DRENDERSTATETYPE type, DWORD data) {
        state = static_cast<DWORD>(type); value = data; return Result();
    }
    HRESULT SetTextureStageState(DWORD slot, D3DTEXTURESTAGESTATETYPE type, DWORD data) {
        stage = slot; state = static_cast<DWORD>(type); value = data; return Result();
    }
    HRESULT SetSamplerState(DWORD slot, D3DSAMPLERSTATETYPE type, DWORD data) {
        stage = slot; state = static_cast<DWORD>(type); value = data; return Result();
    }
    HRESULT SetTexture(DWORD slot, Texture* data) {
        stage = slot; texture = data; return Result();
    }
    HRESULT SetTransform(D3DTRANSFORMSTATETYPE type, const D3DMATRIX* matrix) {
        state = static_cast<DWORD>(type); world = *matrix; return Result();
    }
};
struct Cache {
    unsigned int render[256]{};
    unsigned int stages[8][29]{};
    unsigned int samplers[8][14]{};
    Texture* textures[8]{};
    HRESULT Initialize(Device* device, const D3DMATRIX& world) {
        return render_state_cache::Initialize(device, world, render, stages, samplers);
    }
    bool Unknown() const {
        for (auto data : render) if (data != render_state_cache::UnknownValue) return false;
        for (auto& stage : stages)
            for (auto data : stage) if (data != render_state_cache::UnknownValue) return false;
        for (auto& stage : samplers)
            for (auto data : stage) if (data != render_state_cache::UnknownValue) return false;
        return true;
    }
};
}

int RunRenderStateCacheTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) { ++failures; std::fprintf(stderr, "RenderStateCache: %s\n", name); }
    };
    using namespace render_state_cache;
    D3DLIGHT9 iconLights[2]{};
    for (int index = 0; index < 2; ++index) {
        iconLights[index].Type = D3DLIGHT_DIRECTIONAL;
        iconLights[index].Diffuse = { 0.05f, 0.1f, 0.15f, 0.25f };
        iconLights[index].Specular = { 0.0f, 0.0f, 0.0f, 0.75f };
        iconLights[index].Ambient = { 0.2f, 0.3f, 0.4f, 0.5f };
    }
    native_inventory_lighting::Prepare(iconLights);
    for (int index = 0; index < 2; ++index) {
        const float expected = index == 0 ? 1.0f : 0.6f;
        check(iconLights[index].Diffuse.r == expected && iconLights[index].Diffuse.g == expected &&
            iconLights[index].Diffuse.b == expected && iconLights[index].Specular.r == expected &&
            iconLights[index].Specular.g == expected && iconLights[index].Specular.b == expected,
            "inventory RGB uses native fixed lighting instead of dark world colors");
        check(iconLights[index].Diffuse.a == 0.25f && iconLights[index].Specular.a == 0.75f &&
            iconLights[index].Ambient.r == 0.2f && iconLights[index].Type == D3DLIGHT_DIRECTIONAL,
            "inventory lighting preserves alpha and unrelated light fields");
    }
    check(native_inventory_lighting::AlphaReference(32) == 0xAA &&
        native_inventory_lighting::AlphaReference(16) == 0xFF,
        "native inventory transparency cutoff retains translucent texture detail");
    check(native_inventory_lighting::Ambient == 0xFFFFFFFF && native_inventory_lighting::RenderBlock != 3,
        "inventory ambient is native white without replacing the port's 2D block");
    Device device;
    Cache cache;
    D3DMATRIX identity{};
    identity._11 = identity._22 = identity._33 = identity._44 = 1.0f;
    check(cache.Initialize(&device, identity) == S_OK && cache.Unknown(), "all numeric entries invalidated");
    check(device.state == D3DTS_WORLD && std::memcmp(&identity, &device.world, sizeof(identity)) == 0,
        "world transform passed unchanged");
    device.status = D3DERR_INVALIDCALL;
    cache.render[0] = 99;
    check(cache.Initialize(&device, identity) == D3DERR_INVALIDCALL && cache.Unknown(), "transform failure propagated");
    cache.samplers[7][13] = 99;
    check(cache.Initialize(nullptr, identity) == E_POINTER && cache.Unknown(), "null device invalidates cache safely");
    device.status = S_OK;
    device.calls = 0;

    // Every addressable entry uses a rejecting call, successful retry and cache hit.
    // Index zero is a legacy cache boundary, not a claim that D3D accepts that enum.
    for (DWORD state = 0; state < 256; ++state) {
        const auto type = static_cast<D3DRENDERSTATETYPE>(state);
        device.status = D3DERR_DEVICELOST;
        check(SetRender(&device, cache.render, type, 42) == D3DERR_DEVICELOST && cache.render[state] == UnknownValue,
            "render failure preserves unknown cache");
        device.status = S_OK;
        const auto before = device.calls;
        check(SetRender(&device, cache.render, type, 42) == S_OK && cache.render[state] == 42 &&
            device.calls == before + 1 && device.state == state && device.value == 42, "render retry submitted");
        check(SetRender(&device, cache.render, type, 42) == S_OK && device.calls == before + 1, "render cache hit");
    }
    for (DWORD stage = 0; stage < 8; ++stage) {
        for (DWORD type = 0; type < 29; ++type) {
            const auto native = static_cast<D3DTEXTURESTAGESTATETYPE>(type);
            device.status = E_FAIL;
            check(SetStage(&device, cache.stages, stage, native, 3) == E_FAIL && cache.stages[stage][type] == UnknownValue,
                "stage failure preserves unknown cache");
            device.status = S_FALSE;
            const auto before = device.calls;
            check(SetStage(&device, cache.stages, stage, native, 3) == S_FALSE && cache.stages[stage][type] == 3 &&
                device.stage == stage && device.state == type && device.value == 3, "stage success HRESULT accepted");
            check(SetStage(&device, cache.stages, stage, native, 3) == S_OK && device.calls == before + 1, "stage cache hit");
        }
        for (DWORD type = 0; type < 14; ++type) {
            const auto native = static_cast<D3DSAMPLERSTATETYPE>(type);
            device.status = E_FAIL;
            check(SetSampler(&device, cache.samplers, stage, native, 7) == E_FAIL && cache.samplers[stage][type] == UnknownValue,
                "sampler failure preserves unknown cache");
            device.status = S_OK;
            const auto before = device.calls;
            check(SetSampler(&device, cache.samplers, stage, native, 7) == S_OK && cache.samplers[stage][type] == 7 &&
                device.stage == stage && device.state == type && device.value == 7, "sampler retry submitted");
            check(SetSampler(&device, cache.samplers, stage, native, 7) == S_OK && device.calls == before + 1, "sampler cache hit");
        }
    }
    const auto beforeInvalid = device.calls;
    for (const int invalid : {-1, 256})
        check(SetRender(&device, cache.render, static_cast<D3DRENDERSTATETYPE>(invalid), 0) == E_INVALIDARG, "render bounds");
    for (const int invalid : {-1, 29})
        check(SetStage(&device, cache.stages, 0, static_cast<D3DTEXTURESTAGESTATETYPE>(invalid), 0) == E_INVALIDARG, "stage type bounds");
    for (const int invalid : {-1, 14})
        check(SetSampler(&device, cache.samplers, 0, static_cast<D3DSAMPLERSTATETYPE>(invalid), 0) == E_INVALIDARG, "sampler type bounds");
    for (const DWORD invalid : {DWORD(8), UnknownValue}) {
        check(SetStage(&device, cache.stages, invalid, D3DTSS_COLOROP, 0) == E_INVALIDARG, "stage slot bounds");
        check(SetSampler(&device, cache.samplers, invalid, D3DSAMP_MINFILTER, 0) == E_INVALIDARG, "sampler slot bounds");
        check(SetTexture(&device, cache.textures, invalid, static_cast<Texture*>(nullptr)) == E_INVALIDARG, "texture slot bounds");
    }
    check(device.calls == beforeInvalid, "out of range never submitted");
    check(SetRender(static_cast<Device*>(nullptr), cache.render, D3DRS_ZENABLE, 1) == E_POINTER, "missing render device");
    check(SetStage(static_cast<Device*>(nullptr), cache.stages, 0, D3DTSS_COLOROP, 3) == E_POINTER, "missing stage device");
    check(SetSampler(static_cast<Device*>(nullptr), cache.samplers, 0, D3DSAMP_MINFILTER, 7) == E_POINTER, "missing sampler device");
    check(SetTexture(static_cast<Device*>(nullptr), cache.textures, 0, static_cast<Texture*>(nullptr)) == E_POINTER, "missing texture device");

    const auto renderBefore = cache.render[D3DRS_FOGCOLOR];
    const auto stageBefore = cache.stages[0][D3DTSS_COLOROP];
    const auto samplerBefore = cache.samplers[0][D3DSAMP_MINFILTER];
    device.status = E_FAIL;
    check(FAILED(SetRender(&device, cache.render, D3DRS_FOGCOLOR, 9)) && cache.render[D3DRS_FOGCOLOR] == renderBefore,
        "rejected change preserves last accepted render value");
    check(FAILED(SetStage(&device, cache.stages, 0, D3DTSS_COLOROP, 9)) && cache.stages[0][D3DTSS_COLOROP] == stageBefore,
        "rejected change preserves last accepted stage value");
    check(FAILED(SetSampler(&device, cache.samplers, 0, D3DSAMP_MINFILTER, 9)) && cache.samplers[0][D3DSAMP_MINFILTER] == samplerBefore,
        "rejected change preserves last accepted sampler value");
    device.status = S_OK;
    cache.Initialize(&device, identity);
    const auto beforeWhite = device.calls;
    for (int repeat = 0; repeat < 2; ++repeat) {
        check(SetRender(&device, cache.render, D3DRS_FOGCOLOR, UnknownValue) == S_OK, "all-bits render value accepted");
        check(SetStage(&device, cache.stages, 0, D3DTSS_COLORARG1, UnknownValue) == S_OK, "all-bits stage parameter submitted");
        check(SetSampler(&device, cache.samplers, 0, D3DSAMP_BORDERCOLOR, UnknownValue) == S_OK, "all-bits sampler value accepted");
    }
    check(device.calls == beforeWhite + 6, "sentinel collision never suppresses white-color submission");

    Texture first, second;
    ForgetTextures(cache.textures);
    for (DWORD stage = 0; stage < 8; ++stage) {
        device.status = E_FAIL;
        check(SetTexture(&device, cache.textures, stage, &first) == E_FAIL && cache.textures[stage] == UnknownTexture<Texture>(),
            "rejected texture keeps unknown binding");
        device.status = S_OK;
        const auto before = device.calls;
        check(SetTexture(&device, cache.textures, stage, &first) == S_OK && cache.textures[stage] == &first &&
            device.texture == &first && device.stage == stage, "texture retry submitted");
        check(SetTexture(&device, cache.textures, stage, &first) == S_OK && device.calls == before + 1, "texture cache hit");
        device.status = E_FAIL;
        check(SetTexture(&device, cache.textures, stage, &second) == E_FAIL && cache.textures[stage] == &first,
            "rejected texture replacement preserves old binding");
        device.status = S_OK;
        check(SetTexture(&device, cache.textures, stage, &second) == S_OK && cache.textures[stage] == &second, "texture replacement retry");
    }
    const auto beforeToken = device.calls;
    check(SetTexture(&device, cache.textures, 0, UnknownTexture<Texture>()) == E_INVALIDARG && device.calls == beforeToken,
        "unknown comparison token never submitted");
    check(UnbindTextures(&device, cache.textures) == S_OK && device.calls == beforeToken + 8, "unbind all eight supported stages");
    for (auto texture : cache.textures) check(texture == nullptr, "successful unbind cache synchronized");
    const auto beforeRebind = device.calls;
    check(SetTexture(&device, cache.textures, 7, &second) == S_OK && device.calls == beforeRebind + 1,
        "same pre-reset pointer must be rebound");
    for (int failedSlot = 0; failedSlot < 8; ++failedSlot) {
        device.calls = 0;
        device.failAt = failedSlot + 1;
        check(UnbindTextures(&device, cache.textures) == D3DERR_DEVICELOST && device.calls == failedSlot + 1,
            "unbind stops on first rejected slot");
        for (int slot = 0; slot < 8; ++slot)
            check(cache.textures[slot] == (slot < failedSlot ? nullptr : UnknownTexture<Texture>()),
                "partially cleared binding cache remains truthful");
        device.failAt = 0;
        const auto beforeRetry = device.calls;
        check(SetTexture(&device, cache.textures, static_cast<DWORD>(failedSlot), static_cast<Texture*>(nullptr)) == S_OK &&
            device.calls == beforeRetry + 1, "failed null binding retry not suppressed");
        check(UnbindTextures(&device, cache.textures) == S_OK, "complete unbind retry");
    }
    check(UnbindTextures(static_cast<Device*>(nullptr), cache.textures) == E_POINTER, "null device unbind rejected");
    for (auto texture : cache.textures) check(texture == UnknownTexture<Texture>(), "null device leaves bindings unknown");
    return failures;
}
