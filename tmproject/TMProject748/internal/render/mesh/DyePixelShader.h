#pragma once

#include <d3d9.h>
#include <d3dx9shader.h>
#include <cstring>

namespace dye_pixel_shader
{
// Compatible rendering correction, not the native signed DOTPRODUCT3 formula.
// Average unsigned albedo retains dark cloth detail. Clamp density BEFORE tint
// so strong lighting cannot wash a colored dye out to white. A twofold
// colored-dye gain retains cloth contrast; black keeps its original gain.
// Expand palette chroma by 25% around its average without increasing the
// diffuse light gain. The separate silver streak retains its own color.
// Preserve base UVs and alpha.
// The fixed palette sample supplies the armor's dye hue. Compare the native
// grade texture against grade zero at the SAME animated environment UV. Only
// the positive, high-contrast difference forms the moving silver streak;
// the remaining surface keeps exactly the matte dye color.
// Do not alter the matte base when refinement is zero.
// Black retains its dark graphite palette and ordinary density gain.
// Keep the 1.x pixel pipeline so the existing vertex fog remains applied.
inline constexpr char Source[] =
    "ps.1.4\n"
    "def c2, 0.5, 0.5, 0.13, 0.25\n"
    "def c3, 0.78, 0.80, 0.82, 0\n"
    "texld r1, t1\n"
    "texld r2, t1\n"
    "sub_sat r4.rgb, r2, r1\n"
    "dp3 r4.rgb, r4, c1\n"
    "sub_sat r4.rgb, r4, c2.z\n"
    "mul_x4_sat r4.rgb, r4, c2.x\n"
    "mul r4.rgb, r4, c1.a\n"
    "mov r3, c2\n"
    "phase\n"
    "texld r0, t0\n"
    "texld r3, r3\n"
    "dp3 r1.rgb, r0, c1\n"
    "dp3 r2.rgb, v0, c0\n"
    "mul_x4_sat r1.rgb, r1, r2\n"
    "dp3 r2.rgb, r3, c1\n"
    "sub r2.rgb, r3, r2\n"
    "mad_sat r3.rgb, r2, c2.w, r3\n"
    "mul r0.rgb, r1, r3\n"
    "mad_sat r0.rgb, r4, c3, r0\n";

// Reflection adds at most 90% of the silver highlight in the mask's brightest
// areas; diffuse dyed armor remains underneath. Strength: +1 3.2%, +5 20%,
// +9 43.2%, +15 90%; preserve +13..+15.
inline float SheenStrength(int refinement)
{
    if (refinement < 0) refinement = 0;
    if (refinement > 15) refinement = 15;
    const float level = refinement / 15.0f;
    return 0.90f * level * (0.5f + 0.5f * level);
}

class Shader
{
public:
    Shader() = default;
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    ~Shader() { Reset(); }

    void Reset()
    {
        if (shader_) shader_->Release();
        shader_ = nullptr;
        attempted_ = false;
    }

    IDirect3DPixelShader9* Get(IDirect3DDevice9* device)
    {
        if (attempted_) return shader_;
        attempted_ = true;
        ID3DXBuffer* code = nullptr;
        ID3DXBuffer* errors = nullptr;
        const HRESULT result = D3DXAssembleShader(Source, static_cast<UINT>(std::strlen(Source)),
            nullptr, nullptr, 0, &code, &errors);
        if (SUCCEEDED(result) && code)
            device->CreatePixelShader(static_cast<const DWORD*>(code->GetBufferPointer()), &shader_);
        if (code) code->Release();
        if (errors) errors->Release();
        return shader_;
    }

private:
    IDirect3DPixelShader9* shader_ = nullptr;
    bool attempted_ = false;
};

// Bind only around the armor draw, never around the selection outline or the
// next mesh. Restore even on a failed draw; the device owns shader lifetime.
class Binding
{
public:
    Binding(IDirect3DDevice9* device, Shader& shader, short legend, int refinement, bool enabled)
        : device_(device)
    {
        if (!enabled) return;
        auto* program = shader.Get(device);
        if (!program) return; // Retain the legacy stages on unsupported devices.
        if (FAILED(device->GetPixelShader(&previous_))) return;
        if (FAILED(device->GetPixelShaderConstantF(0, constants_, 2))) return;
        restore_ = true;
        const float lightWeight = legend == 120 ? 1.0f / 12 : 1.0f / 6;
        const float constants[8] = {lightWeight, lightWeight, lightWeight, 0,
            1.0f / 3, 1.0f / 3, 1.0f / 3, SheenStrength(refinement)};
        if (SUCCEEDED(device->SetPixelShaderConstantF(0, constants, 2)))
            active_ = SUCCEEDED(device->SetPixelShader(program));
    }
    ~Binding()
    {
        if (restore_)
        {
            device_->SetPixelShader(previous_);
            device_->SetPixelShaderConstantF(0, constants_, 2);
        }
        if (previous_) previous_->Release();
    }
    Binding(const Binding&) = delete;
    Binding& operator=(const Binding&) = delete;
    bool Active() const { return active_; }

private:
    IDirect3DDevice9* device_;
    IDirect3DPixelShader9* previous_ = nullptr;
    float constants_[8]{};
    bool restore_ = false;
    bool active_ = false;
};

// The shader needs the unrefined effect, its grade-specific counterpart and
// the actual dye palette. Do not leak these extra samplers into later meshes.
class TextureBinding
{
public:
    TextureBinding(IDirect3DDevice9* device, IDirect3DBaseTexture9* unrefined,
        IDirect3DBaseTexture9* refined, IDirect3DBaseTexture9* palette)
        : device_(device)
    {
        IDirect3DBaseTexture9* textures[3] = {unrefined, refined, palette};
        for (DWORD stage = 1; stage <= 3; ++stage)
        {
            if (FAILED(device_->GetTexture(stage, &previous_[stage - 1]))) return;
        }
        for (DWORD stage = 1; stage <= 3; ++stage)
        {
            if (FAILED(device_->SetTexture(stage, textures[stage - 1])))
            {
                for (DWORD restore = 1; restore < stage; ++restore)
                    device_->SetTexture(restore, previous_[restore - 1]);
                return;
            }
        }
        active_ = true;
    }
    ~TextureBinding()
    {
        if (active_)
            for (DWORD stage = 1; stage <= 3; ++stage)
                device_->SetTexture(stage, previous_[stage - 1]);
        for (auto* texture : previous_)
            if (texture) texture->Release();
    }
    TextureBinding(const TextureBinding&) = delete;
    TextureBinding& operator=(const TextureBinding&) = delete;
    bool Active() const { return active_; }

private:
    IDirect3DDevice9* device_;
    IDirect3DBaseTexture9* previous_[3]{};
    bool active_ = false;
};
}
