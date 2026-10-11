#pragma once
#include <d3d9.h>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace render_state_cache {
constexpr DWORD UnknownValue = static_cast<DWORD>(-1);

template<class Value, class Submit>
HRESULT Update(Value& cached, Value requested, bool unknown, Submit submit)
{
    if (!unknown && cached == requested) return S_OK;
    const HRESULT result = submit();
    if (SUCCEEDED(result)) cached = requested;
    return result;
}

template<class Texture>
Texture* UnknownTexture()
{
    // Non-owning comparison token only: never dereference, release or submit it.
    return reinterpret_cast<Texture*>((std::numeric_limits<std::uintptr_t>::max)());
}

template<class Device, class Value, std::size_t States>
HRESULT SetRender(Device* device, Value (&cache)[States], D3DRENDERSTATETYPE state, DWORD value)
{
    static_assert(std::is_unsigned_v<Value> && sizeof(Value) == sizeof(DWORD), "32-bit unsigned state cache");
    const auto index = static_cast<unsigned int>(state);
    if (index >= States) return E_INVALIDARG;
    if (!device) return E_POINTER;
    return Update(cache[index], static_cast<Value>(value), cache[index] == UnknownValue,
        [&] { return device->SetRenderState(state, value); });
}

template<class Device, class Value, std::size_t Stages, std::size_t Types>
HRESULT SetStage(Device* device, Value (&cache)[Stages][Types], DWORD stage,
    D3DTEXTURESTAGESTATETYPE type, DWORD value)
{
    static_assert(std::is_unsigned_v<Value> && sizeof(Value) == sizeof(DWORD), "32-bit unsigned stage cache");
    const auto index = static_cast<unsigned int>(type);
    if (stage >= Stages || index >= Types) return E_INVALIDARG;
    if (!device) return E_POINTER;
    return Update(cache[stage][index], static_cast<Value>(value), cache[stage][index] == UnknownValue,
        [&] { return device->SetTextureStageState(stage, type, value); });
}

template<class Device, class Value, std::size_t Stages, std::size_t Types>
HRESULT SetSampler(Device* device, Value (&cache)[Stages][Types], DWORD stage,
    D3DSAMPLERSTATETYPE state, DWORD value)
{
    static_assert(std::is_unsigned_v<Value> && sizeof(Value) == sizeof(DWORD), "32-bit unsigned sampler cache");
    const auto index = static_cast<unsigned int>(state);
    if (stage >= Stages || index >= Types) return E_INVALIDARG;
    if (!device) return E_POINTER;
    return Update(cache[stage][index], static_cast<Value>(value), cache[stage][index] == UnknownValue,
        [&] { return device->SetSamplerState(stage, state, value); });
}

template<class Device, class Texture, std::size_t Stages>
HRESULT SetTexture(Device* device, Texture* (&cache)[Stages], DWORD stage, Texture* texture)
{
    if (stage >= Stages || texture == UnknownTexture<Texture>()) return E_INVALIDARG;
    if (!device) return E_POINTER;
    return Update(cache[stage], texture, cache[stage] == UnknownTexture<Texture>(),
        [&] { return device->SetTexture(stage, texture); });
}

template<class Texture, std::size_t Stages>
void ForgetTextures(Texture* (&cache)[Stages])
{
    for (auto& texture : cache) texture = UnknownTexture<Texture>();
}

template<class Device, class Texture, std::size_t Stages>
HRESULT UnbindTextures(Device* device, Texture* (&cache)[Stages])
{
    ForgetTextures(cache);
    for (DWORD stage = 0; stage < Stages; ++stage) {
        const HRESULT result = SetTexture(device, cache, stage, static_cast<Texture*>(nullptr));
        if (FAILED(result)) return result;
    }
    return S_OK;
}

template<class Device, class Value, std::size_t States, std::size_t Stages, std::size_t Types,
    std::size_t SamplerStages, std::size_t SamplerTypes>
HRESULT Initialize(Device* device, const D3DMATRIX& world, Value (&states)[States],
    Value (&stageStates)[Stages][Types], Value (&samplers)[SamplerStages][SamplerTypes])
{
    static_assert(std::is_unsigned_v<Value> && sizeof(Value) == sizeof(DWORD), "32-bit unsigned device cache");
    for (auto& state : states) state = UnknownValue;
    for (auto& stage : stageStates)
        for (auto& state : stage) state = UnknownValue;
    for (auto& stage : samplers)
        for (auto& state : stage) state = UnknownValue;
    if (!device) return E_POINTER;
    return device->SetTransform(D3DTS_WORLD, &world);
}
}
