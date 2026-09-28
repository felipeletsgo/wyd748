#include "../internal/render/mesh/DyeTextureStages.h"
#include "../internal/render/mesh/DyePixelShader.h"
#include <d3dx9tex.h>
#include <wrl/client.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <fstream>
#include <iterator>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace
{
void Require(HRESULT result)
{
    if (FAILED(result))
    {
        std::printf("Direct3D call failed: 0x%08lX\n", static_cast<unsigned long>(result));
        throw std::runtime_error("Device test could not complete");
    }
}

struct DeviceStates
{
    IDirect3DDevice9* device;
    void SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE state, DWORD value)
    {
        Require(device->SetTextureStageState(stage, state, value));
    }
};

ComPtr<IDirect3DTexture9> Texture(IDirect3DDevice9* device, DWORD color)
{
    ComPtr<IDirect3DTexture9> texture;
    Require(device->CreateTexture(1, 1, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED,
        texture.GetAddressOf(), nullptr));
    D3DLOCKED_RECT locked{};
    Require(texture->LockRect(0, &locked, nullptr, 0));
    *static_cast<DWORD*>(locked.pBits) = color;
    Require(texture->UnlockRect(0));
    return texture;
}

float Clamp(float value) { return std::clamp(value, 0.0f, 1.0f); }

float SaturatedDyeChannel(DWORD dye, int shift)
{
    const float gray = static_cast<float>((dye & 255) + ((dye >> 8) & 255)
        + ((dye >> 16) & 255)) / (3.0f * 255);
    const float channel = static_cast<float>((dye >> shift) & 255) / 255.0f;
    return Clamp(gray + 1.25f * (channel - gray));
}

// Independent oracle: literal operations from FUN_004c3eec, not the helper.
DWORD Expected(int base, int light, DWORD dye, short legend, int adapter)
{
    const float texture = base / 255.0f;
    const float diffuse = light / 255.0f;
    const float density = adapter == 0
        ? Clamp(12 * (texture - 0.5f) * (diffuse - 0.5f))
        : texture * diffuse;
    DWORD result = 0;
    for (int shift : {0, 8, 16})
    {
        const float color = ((dye >> shift) & 255) / 255.0f;
        const float output = adapter != 0 ? Clamp(density + color)
            : Clamp(density * color * (legend == 120 ? 1.0f : 4.0f));
        result |= static_cast<DWORD>(std::lround(output * 255)) << shift;
    }
    return result;
}

bool Near(DWORD actual, DWORD expected)
{
    for (int shift : {0, 8, 16})
        if (std::abs(static_cast<int>((actual >> shift) & 255)
            - static_cast<int>((expected >> shift) & 255)) > 4)
            return false;
    return true;
}

DWORD Draw(IDirect3DDevice9* device, IDirect3DSurface9* target,
    IDirect3DSurface9* readback, int light, std::vector<DWORD>* pixels = nullptr,
    float environmentOffset = 0, int fog = 255)
{
    struct Vertex { float x, y, z, rhw; DWORD color, specular; float u0, v0, u1, v1; };
    const DWORD color = D3DCOLOR_XRGB(light, light, light);
    const DWORD specular = D3DCOLOR_ARGB(fog, 0, 0, 0);
    Vertex vertices[] = {
        {-0.5f, -0.5f, 0, 1, color, specular, 0, 0, 0, 0},
        {127.5f, -0.5f, 0, 1, color, specular, 1, 0, 1, 0},
        {-0.5f, 127.5f, 0, 1, color, specular, 0, 1, 0, 1},
        {127.5f,127.5f, 0, 1, color, specular, 1, 1, 1, 1},
    };
    for (auto& vertex : vertices)
    {
        vertex.u1 += environmentOffset;
        vertex.v1 += environmentOffset;
    }
    Require(device->Clear(0, nullptr, D3DCLEAR_TARGET, 0xFFFF00FF, 1, 0));
    Require(device->BeginScene());
    const HRESULT draw = device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(Vertex));
    Require(device->EndScene());
    Require(draw);
    Require(device->GetRenderTargetData(target, readback));
    D3DLOCKED_RECT locked{};
    Require(readback->LockRect(&locked, nullptr, D3DLOCK_READONLY));
    const DWORD pixel = *reinterpret_cast<const DWORD*>(
        static_cast<const unsigned char*>(locked.pBits) + locked.Pitch + sizeof(DWORD));
    if (pixels)
    {
        pixels->clear();
        for (int y = 0; y < 128; ++y)
        {
            const auto* row = reinterpret_cast<const DWORD*>(
                static_cast<const unsigned char*>(locked.pBits) + y * locked.Pitch);
            pixels->insert(pixels->end(), row, row + 128);
        }
    }
    Require(readback->UnlockRect());
    return pixel;
}

ComPtr<IDirect3DTexture9> LoadWys(IDirect3DDevice9* device, const char* path)
{
    std::ifstream file(path, std::ios::binary);
    std::vector<char> bytes{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    if (bytes.size() < 129) throw std::runtime_error("Missing WYS fixture; run from the repository root");
    // Same header reconstruction as TextureManager::LoadModelTexture.
    std::memcpy(bytes.data() + 1, "DDS", 3);
    std::memcpy(bytes.data() + 85, bytes[85] == '2' ? "DXT1" : "DXT3", 4);
    ComPtr<IDirect3DTexture9> texture;
    Require(D3DXCreateTextureFromFileInMemoryEx(device, bytes.data() + 1,
        static_cast<UINT>(bytes.size() - 1), D3DX_DEFAULT, D3DX_DEFAULT, 1, 0,
        D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_FILTER_NONE,
        0, nullptr, nullptr, texture.GetAddressOf()));
    return texture;
}

std::vector<DWORD> Texels(IDirect3DTexture9* texture)
{
    D3DSURFACE_DESC desc{};
    Require(texture->GetLevelDesc(0, &desc));
    D3DLOCKED_RECT locked{};
    Require(texture->LockRect(0, &locked, nullptr, D3DLOCK_READONLY));
    std::vector<DWORD> pixels;
    for (UINT y = 0; y < desc.Height; ++y)
    {
        const auto* row = reinterpret_cast<const DWORD*>(
            static_cast<const unsigned char*>(locked.pBits) + y * locked.Pitch);
        pixels.insert(pixels.end(), row, row + desc.Width);
    }
    Require(texture->UnlockRect(0));
    return pixels;
}

void CheckSheen(IDirect3DDevice9* device, IDirect3DSurface9* target,
    IDirect3DSurface9* readback, dye_pixel_shader::Shader& shader,
    const std::vector<DWORD>& base, IDirect3DTexture9* dyeAsset,
    const char* dyePath, short legend, int light, int& checks, bool uniformBase = false)
{
    const auto palette = Texels(dyeAsset);
    D3DSURFACE_DESC paletteDesc{};
    Require(dyeAsset->GetLevelDesc(0, &paletteDesc));
    const DWORD tint = palette[(paletteDesc.Height / 2) * paletteDesc.Width + paletteDesc.Width / 2];
    // Yellow's native grade textures are solid; its moving mask comes from
    // the silver family, while the yellow palette remains unchanged.
    const std::string maskPrefix = legend == 124 ? "tmproject/client748/Effect/si"
        : std::string(dyePath).substr(0, std::strlen(dyePath) - 8);
    std::vector<ComPtr<IDirect3DTexture9>> gradeTextures;
    std::vector<std::vector<DWORD>> gradePixels;
    for (int grade = 0; grade < 12; ++grade)
    {
        char suffix[16]{};
        std::snprintf(suffix, sizeof(suffix), "%04d.wys", grade);
        gradeTextures.push_back(LoadWys(device, (maskPrefix + suffix).c_str()));
        gradePixels.push_back(Texels(gradeTextures.back().Get()));
    }
    D3DSURFACE_DESC maskDesc{};
    Require(gradeTextures.front()->GetLevelDesc(0, &maskDesc));
    std::vector<DWORD> firstPhase;
    for (float phase : {0.0f, 0.37f, 0.83f})
    {
        std::vector<DWORD> phaseProbe;
        for (int refinement = 0; refinement <= 15; ++refinement)
        {
            const int grade = refinement == 0 ? 0 : (std::min)(refinement - 1, 11);
            std::vector<DWORD> output;
            {
                dye_pixel_shader::Binding binding(device, shader, legend, refinement, true);
                if (!binding.Active()) throw std::runtime_error("Refinement shader did not bind");
                dye_pixel_shader::TextureBinding textures(device, gradeTextures.front().Get(),
                    gradeTextures[grade].Get(), dyeAsset);
                if (!textures.Active()) throw std::runtime_error("Refinement textures did not bind");
                Draw(device, target, readback, light, &output, phase);
                if (refinement == 15)
                {
                    Require(device->SetRenderState(D3DRS_FOGCOLOR, D3DCOLOR_XRGB(20, 40, 60)));
                    Require(device->SetRenderState(D3DRS_FOGENABLE, TRUE));
                    std::vector<DWORD> fogged;
                    Draw(device, target, readback, light, &fogged, phase, 128);
                    Require(device->SetRenderState(D3DRS_FOGENABLE, FALSE));
                    for (size_t i = 0; i < output.size(); ++i)
                    {
                        DWORD expected = 0;
                        for (int shift : {0, 8, 16})
                        {
                            const int foreground = (output[i] >> shift) & 255;
                            const int background = (D3DCOLOR_XRGB(20, 40, 60) >> shift) & 255;
                            expected |= static_cast<DWORD>(std::lround(
                                (foreground * 128.0f + background * 127.0f) / 255)) << shift;
                        }
                        if (!Near(fogged[i], expected))
                            throw std::runtime_error("Refined dye bypassed vertex fog");
                        ++checks;
                    }
                }
            }
            // TextureBinding must restore every sampler used by the extra pass.
            for (DWORD stage = 1; stage <= 3; ++stage)
            {
                ComPtr<IDirect3DBaseTexture9> restored;
                Require(device->GetTexture(stage, restored.GetAddressOf()));
                if (restored.Get() != (stage == 1 ? dyeAsset : nullptr))
                    throw std::runtime_error("Refinement sampler state leaked to another mesh");
            }
            int mattePixels = 0;
            int streakPixels = 0;
            for (size_t i = 0; i < base.size(); ++i)
            {
                const float u = (static_cast<float>(i % 128) + 0.5f) / 128 + phase;
                const float v = (static_cast<float>(i / 128) + 0.5f) / 128 + phase;
                const auto x = static_cast<UINT>(u * maskDesc.Width) % maskDesc.Width;
                const auto y = static_cast<UINT>(v * maskDesc.Height) % maskDesc.Height;
                const DWORD neutral = gradePixels.front()[y * maskDesc.Width + x];
                const DWORD refined = gradePixels[grade][y * maskDesc.Width + x];
                float difference = 0;
                for (int shift : {0, 8, 16})
                    difference += (std::max)(0, static_cast<int>((refined >> shift) & 255)
                        - static_cast<int>((neutral >> shift) & 255)) / (3.0f * 255);
                const float level = refinement / 15.0f;
                const float strength = 0.45f * level + 0.45f * level * level;
                const int sum = (base[i] & 255) + ((base[i] >> 8) & 255) + ((base[i] >> 16) & 255);
                const float density = sum / (3.0f * 255) * (light / 255.0f)
                    * (legend == 120 ? 1.0f : 2.0f);
                const float reflection = strength * Clamp(2.0f * (difference - 0.13f));
                DWORD expected = 0;
                int largestLift = 0;
                for (int shift : {0, 8, 16})
                {
                    const int color = (output[i] >> shift) & 255;
                    const float dye = SaturatedDyeChannel(tint, shift);
                    const float silver = shift == 0 ? 0.82f : (shift == 8 ? 0.80f : 0.78f);
                    expected |= static_cast<DWORD>(std::lround(255 * Clamp(
                        dye * Clamp(density) + silver * reflection))) << shift;
                    const float baseline = dye * Clamp(density) * 255;
                    if (color + 3 < baseline || color > baseline + silver * 255 * 0.90f + 3
                        || (difference <= 0.13f && color > baseline + 3))
                    {
                        std::printf("Streak bound: legend=%d grade=%d pixel=%zu channel=%d color=%d base=%.1f difference=%.3f\n",
                            legend, refinement, i, shift, color, baseline, difference);
                        throw std::runtime_error("Refinement changed the matte area or exceeded the silver cap");
                    }
                    largestLift = (std::max)(largestLift, color - static_cast<int>(baseline));
                }
                if (!Near(output[i], expected)
                    || std::abs(static_cast<int>(output[i] >> 24) - static_cast<int>(base[i] >> 24)) > 1)
                {
                    std::printf("Streak mismatch: legend=%d light=%d refinement=%d phase=%.2f pixel=%zu actual=%08lx expected=%08lx density=%.4f difference=%.4f\n",
                        legend, light, refinement, phase, i, output[i], expected, density, difference);
                    throw std::runtime_error("Native grade streak, dyed detail, or alpha mismatch");
                }
                if (largestLift <= 3) ++mattePixels;
                if (largestLift >= 8) ++streakPixels;
                ++checks;
            }
            if (refinement == 9) phaseProbe = output;
            if (refinement == 9 && !uniformBase)
            {
                std::printf("Grade +9 legend=%d phase=%.2f: %d streak pixels, %d matte pixels\n",
                    legend, phase, streakPixels, mattePixels);
                if (streakPixels < 64 || mattePixels < static_cast<int>(output.size() / 2))
                    throw std::runtime_error("Grade +9 must show a narrow streak without washing the armor");
            }
            if (refinement == 0 && mattePixels != static_cast<int>(output.size()))
                throw std::runtime_error("Unrefined dye acquired a moving streak");
        }
        if (firstPhase.empty()) firstPhase = phaseProbe;
        else if (phaseProbe == firstPhase)
            throw std::runtime_error("Refinement streak never moved with the environment UVs");
    }
    std::printf("Streak legend=%d light=%d: +0..+15, moving grade texture/matte base/fog PASS\n", legend, light);
}

void CheckArmor(IDirect3DDevice9* device, IDirect3DSurface9* target,
    IDirect3DSurface9* readback, int& checks)
{
    ID3DXBuffer* assembled = nullptr;
    ID3DXBuffer* errors = nullptr;
    const HRESULT assembly = D3DXAssembleShader(dye_pixel_shader::Source,
        static_cast<UINT>(std::strlen(dye_pixel_shader::Source)), nullptr, nullptr,
        0, &assembled, &errors);
    if (FAILED(assembly))
    {
        const std::string message = errors ? static_cast<const char*>(errors->GetBufferPointer())
            : "Unknown dye pixel shader assembly failure";
        if (assembled) assembled->Release();
        if (errors) errors->Release();
        throw std::runtime_error(message);
    }
    if (assembled) assembled->Release();
    if (errors) errors->Release();
    dye_pixel_shader::Shader shader;
    const auto armor = LoadWys(device, "tmproject/client748/mesh/ch010203.wys");
    const auto base = Texels(armor.Get());
    if (base.size() != 128 * 128) throw std::runtime_error("Unexpected armor fixture dimensions");
    struct Dye { short legend; const char* file; };
    for (const Dye fixture : {
        Dye{116, "tmproject/client748/Effect/bl0000.wys"},
        Dye{117, "tmproject/client748/Effect/re0000.wys"},
        Dye{118, "tmproject/client748/Effect/gr0000.wys"},
        Dye{119, "tmproject/client748/Effect/si0000.wys"},
        Dye{120, "tmproject/client748/Effect/dk0000.wys"},
        Dye{121, "tmproject/client748/Effect/vi0000.wys"},
        Dye{122, "tmproject/client748/Effect/or0000.wys"},
        Dye{123, "tmproject/client748/Effect/pi0000.wys"},
        Dye{124, "tmproject/client748/Effect/ye0000.wys"},
        Dye{125, "tmproject/client748/Effect/db0000.wys"}})
    {
        const auto dyeAsset = LoadWys(device, fixture.file);
        D3DSURFACE_DESC dyeDesc{};
        Require(dyeAsset->GetLevelDesc(0, &dyeDesc));
        const DWORD dyeColor = Texels(dyeAsset.Get())[
            (dyeDesc.Height / 2) * dyeDesc.Width + dyeDesc.Width / 2];
        std::printf("Dye legend=%d center=%08lx dimensions=%ux%u\n",
            fixture.legend, dyeColor, dyeDesc.Width, dyeDesc.Height);
        const int maskShift = fixture.legend == 116 || fixture.legend == 121
            || fixture.legend == 125 ? 0 : (fixture.legend == 118 || fixture.legend == 124 ? 8 : 16);
        int minMask = 255, maxMask = 0;
        for (DWORD pixel : Texels(dyeAsset.Get()))
        {
            const int mask = (pixel >> maskShift) & 255;
            minMask = (std::min)(minMask, mask);
            maxMask = (std::max)(maxMask, mask);
        }
        std::printf("Dye legend=%d mask channel=%d..%d of 255\n", fixture.legend, minMask, maxMask);
        Require(device->SetTexture(0, armor.Get()));
        Require(device->SetTexture(1, dyeAsset.Get()));
        for (int light : {140, 255})
        {
            Require(device->SetTexture(2, dyeAsset.Get()));
            Require(device->SetTexture(3, dyeAsset.Get()));
            std::vector<DWORD> output;
            const float sentinel[8] = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f};
            Require(device->SetPixelShaderConstantF(0, sentinel, 2));
            {
                dye_pixel_shader::Binding binding(device, shader, fixture.legend, 0, true);
                if (!binding.Active()) throw std::runtime_error("Dye shader did not bind");
                Draw(device, target, readback, light, &output);
                // Real effect textures must not turn normal/time-dependent UVs
                // into moving gloss. Keep the diffuse base shading unchanged.
                for (float phase : {0.37f, 0.83f})
                {
                    std::vector<DWORD> shifted;
                    Draw(device, target, readback, light, &shifted, phase);
                    if (shifted != output)
                        throw std::runtime_error("Matte dye changed with environment UVs");
                    checks += static_cast<int>(shifted.size());
                }
                constexpr DWORD fogColor = D3DCOLOR_XRGB(20, 40, 60);
                Require(device->SetRenderState(D3DRS_FOGCOLOR, fogColor));
                Require(device->SetRenderState(D3DRS_FOGENABLE, TRUE));
                for (int fog : {0, 128, 255})
                {
                    std::vector<DWORD> fogged;
                    Draw(device, target, readback, light, &fogged, 0, fog);
                    for (size_t i = 0; i < fogged.size(); ++i)
                    {
                        DWORD expected = 0;
                        for (int shift : {0, 8, 16})
                        {
                            const float color = static_cast<float>((output[i] >> shift) & 255);
                            const float background = static_cast<float>((fogColor >> shift) & 255);
                            expected |= static_cast<DWORD>(std::lround(
                                (color * fog + background * (255 - fog)) / 255)) << shift;
                        }
                        if (!Near(fogged[i], expected))
                            throw std::runtime_error("Matte dye bypassed vertex fog");
                        ++checks;
                    }
                }
                Require(device->SetRenderState(D3DRS_FOGENABLE, FALSE));
            }
            ComPtr<IDirect3DPixelShader9> restored;
            Require(device->GetPixelShader(restored.GetAddressOf()));
            float constants[8]{};
            Require(device->GetPixelShaderConstantF(0, constants, 2));
            if (restored || std::memcmp(constants, sentinel, sizeof(sentinel)))
                throw std::runtime_error("Dye draw leaked shader state");
            int recovered = 0;
            for (size_t i = 0; i < base.size(); ++i)
            {
                const int sum = (base[i] & 255) + ((base[i] >> 8) & 255) + ((base[i] >> 16) & 255);
                const float intensity = Clamp(sum / (3.0f * 255) * (light / 255.0f)
                    * (fixture.legend == 120 ? 1.0f : 2.0f));
                DWORD expected = 0;
                for (int shift : {0, 8, 16})
                    expected |= static_cast<DWORD>(std::lround(255 * SaturatedDyeChannel(dyeColor, shift)
                        * intensity)) << shift;
                if (!Near(output[i], expected)
                    || std::abs(static_cast<int>(output[i] >> 24) - static_cast<int>(base[i] >> 24)) > 1)
                {
                    std::printf("Armor mismatch legend=%d light=%d index=%zu base=%08lX got=%08lX expected=%06lX\n",
                        fixture.legend, light, i, base[i], output[i], expected);
                    throw std::runtime_error("Real armor pixel mismatch");
                }
                // This entire positive-dark range was zeroed by the previous DOTPRODUCT3.
                if (sum > 60 && sum < 300 && (output[i] & 0xFFFFFF) != 0) ++recovered;
                ++checks;
            }
            if (recovered < 1000) throw std::runtime_error("Dark cloth detail is still clipped");
            std::printf("Armor legend=%d light=%d: %d previously black texels retain detail\n",
                fixture.legend, light, recovered);
            Require(device->SetTexture(2, nullptr));
            Require(device->SetTexture(3, nullptr));
            CheckSheen(device, target, readback, shader, base, dyeAsset.Get(),
                fixture.file, fixture.legend, light, checks);
        }
        // Reproduce the vanished-reflection case explicitly: density saturates
        // across the entire mesh. Contrast must still move and grow with grade.
        constexpr DWORD brightBase = D3DCOLOR_ARGB(137, 255, 255, 255);
        const auto brightArmor = Texture(device, brightBase);
        Require(device->SetTexture(0, brightArmor.Get()));
        CheckSheen(device, target, readback, shader, std::vector<DWORD>(128 * 128, brightBase),
            dyeAsset.Get(), fixture.file, fixture.legend, 255, checks, true);
        // Recreate the same cache used after client device invalidation.
        shader.Reset();
    }
}

int Run(HWND window)
{
    ComPtr<IDirect3D9> d3d;
    d3d.Attach(Direct3DCreate9(D3D_SDK_VERSION));
    if (!d3d) throw std::runtime_error("Direct3D9 unavailable");
    D3DADAPTER_IDENTIFIER9 adapterId{};
    Require(d3d->GetAdapterIdentifier(D3DADAPTER_DEFAULT, 0, &adapterId));
    std::printf("Device: %s\n", adapterId.Description);
    D3DPRESENT_PARAMETERS parameters{};
    parameters.Windowed = TRUE;
    parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
    parameters.BackBufferWidth = parameters.BackBufferHeight = 128;
    parameters.BackBufferFormat = D3DFMT_A8R8G8B8;
    parameters.hDeviceWindow = window;
    ComPtr<IDirect3DDevice9> device;
    Require(d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING, &parameters, device.GetAddressOf()));
    ComPtr<IDirect3DSurface9> target, readback;
    Require(device->GetRenderTarget(0, target.GetAddressOf()));
    Require(device->CreateOffscreenPlainSurface(128, 128, D3DFMT_A8R8G8B8,
        D3DPOOL_SYSTEMMEM, readback.GetAddressOf(), nullptr));
    Require(device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX2));
    Require(device->SetRenderState(D3DRS_LIGHTING, FALSE));
    Require(device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
    Require(device->SetRenderState(D3DRS_ZENABLE, FALSE));
    Require(device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
    Require(device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT));
    Require(device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
    Require(device->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_POINT));
    Require(device->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
    DeviceStates states{device.Get()};
    int checks = 0;
    if (dye_pixel_shader::SheenStrength(-1) != 0
        || dye_pixel_shader::SheenStrength(0) != 0
        || std::abs(dye_pixel_shader::SheenStrength(15) - 0.90f) > 0.00001f
        || std::abs(dye_pixel_shader::SheenStrength(9) - 0.432f) > 0.00001f
        || dye_pixel_shader::SheenStrength(100) != dye_pixel_shader::SheenStrength(15))
        throw std::runtime_error("Invalid refinement strength bounds");
    for (short legend = 116; legend <= 125; ++legend)
        for (int adapter = 0; adapter < 3; ++adapter)
            for (int base : {64, 128, 192, 255})
                for (int light : {140, 255})
                {
                    // Measured blue +1 and black +1 texels; other legends exercise routing.
                    const DWORD dye = legend == 120 ? D3DCOLOR_XRGB(95, 95, 95)
                        : D3DCOLOR_XRGB(24, 82, 186);
                    const auto armor = Texture(device.Get(), D3DCOLOR_XRGB(base, base, base));
                    const auto overlay = Texture(device.Get(), dye);
                    Require(device->SetTexture(0, armor.Get()));
                    Require(device->SetTexture(1, overlay.Get()));
                    dye_texture_stages::Apply(states, legend, 'A', adapter == 1, adapter == 2);
                    const DWORD actual = Draw(device.Get(), target.Get(), readback.Get(), light);
                    const DWORD expected = Expected(base, light, dye, legend, adapter);
                    ++checks;
                    if (!Near(actual, expected))
                    {
                        std::printf("FAIL legend=%d adapter=%d base=%d light=%d actual=%06lX expected=%06lX\n",
                            legend, adapter, base, light, actual & 0xFFFFFF, expected);
                        return 1;
                    }
                    if (legend == 116 && adapter == 0 && base == 192 && light == 140)
                    {
                        // Prove the previous erroneous operations fail the same pixel oracle.
                        states.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MULTIPLYADD);
                        states.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_ADD);
                        if (Near(Draw(device.Get(), target.Get(), readback.Get(), light), expected))
                            throw std::runtime_error("Old whitening regression was not rejected");
                        ++checks;
                    }
                }
    CheckArmor(device.Get(), target.Get(), readback.Get(), checks);
    std::printf("DyeTextureStagesDeviceTests: %d pixel checks PASS; real armor, moving grade streak and shader restoration verified\n", checks);
    return 0;
}
}

int main()
{
    // Hidden test window only; never attach to or manipulate the user's game.
    HWND window = CreateWindowExW(0, L"STATIC", L"Dye stage test", WS_OVERLAPPED,
        0, 0, 16, 16, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
    if (!window) return 1;
    int result = 1;
    try { result = Run(window); }
    catch (const std::exception& error) { std::printf("FAIL: %s\n", error.what()); }
    DestroyWindow(window);
    return result;
}
