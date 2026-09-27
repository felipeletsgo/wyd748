#pragma once

#include <d3d9.h>
#include <d3dx9shader.h>
#include <cstring>

namespace dye_pixel_shader
{
// Compatible rendering correction, not the native signed DOTPRODUCT3 formula.
// Average unsigned albedo retains dark cloth detail. Clamp density BEFORE tint
// so strong lighting cannot wash a colored dye out to white. Keep the existing
// fourfold colored-dye gain and ordinary black-dye gain, UVs, and texture alpha.
inline constexpr char Source[] =
    "ps.1.1\n"
    "tex t0\n"
    "tex t1\n"
    "dp3 r0.rgb, t0, c0\n"
    "dp3 r1.rgb, v0, c0\n"
    "mul r0.rgb, r0, r1\n"
    "mul_x4_sat r0.rgb, r0, c1\n"
    "mul r0.rgb, r0, t1\n"
    "mov r0.a, t0.a\n";

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
    Binding(IDirect3DDevice9* device, Shader& shader, short legend, bool enabled)
        : device_(device)
    {
        if (!enabled) return;
        auto* program = shader.Get(device);
        if (!program) return; // Retain the legacy stages on unsupported devices.
        if (FAILED(device->GetPixelShader(&previous_))) return;
        if (FAILED(device->GetPixelShaderConstantF(0, constants_, 2))) return;
        restore_ = true;
        const float gain = legend == 120 ? 0.25f : 1.0f;
        const float constants[8] = {1.0f / 3, 1.0f / 3, 1.0f / 3, 1,
            gain, gain, gain, gain};
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
}
