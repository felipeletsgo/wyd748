#include "../internal/render/mesh/DyeTextureStages.h"
#include "../internal/render/mesh/DyePixelShader.h"
#include <d3dx9tex.h>
#include <wrl/client.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <stdexcept>
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
    IDirect3DSurface9* readback, int light, std::vector<DWORD>* pixels = nullptr)
{
    struct Vertex { float x, y, z, rhw; DWORD color; float u0, v0, u1, v1; };
    const DWORD color = D3DCOLOR_XRGB(light, light, light);
    const Vertex vertices[] = {
        {-0.5f, -0.5f, 0, 1, color, 0, 0, 0, 0},
        {127.5f, -0.5f, 0, 1, color, 1, 0, 1, 0},
        {-0.5f, 127.5f, 0, 1, color, 0, 1, 0, 1},
        {127.5f,127.5f, 0, 1, color, 1, 1, 1, 1},
    };
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

void CheckArmor(IDirect3DDevice9* device, IDirect3DSurface9* target,
    IDirect3DSurface9* readback, int& checks)
{
    dye_pixel_shader::Shader shader;
    const auto armor = LoadWys(device, "tmproject/client748/mesh/ch010203.wys");
    const auto base = Texels(armor.Get());
    if (base.size() != 128 * 128) throw std::runtime_error("Unexpected armor fixture dimensions");
    struct Dye { short legend; const char* file; };
    for (const Dye fixture : {
        Dye{116, "tmproject/client748/Effect/bl0000.wys"},
        Dye{120, "tmproject/client748/Effect/dk0000.wys"},
        Dye{123, "tmproject/client748/Effect/pi0000.wys"}})
    {
        const auto dyeAsset = LoadWys(device, fixture.file);
        const DWORD dyeColor = Texels(dyeAsset.Get()).front();
        const auto dye = Texture(device, dyeColor);
        Require(device->SetTexture(0, armor.Get()));
        Require(device->SetTexture(1, dye.Get()));
        for (int light : {140, 255})
        {
            std::vector<DWORD> output;
            const float sentinel[8] = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f};
            Require(device->SetPixelShaderConstantF(0, sentinel, 2));
            {
                dye_pixel_shader::Binding binding(device, shader, fixture.legend, true);
                if (!binding.Active()) throw std::runtime_error("Dye shader did not bind");
                Draw(device, target, readback, light, &output);
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
                    * (fixture.legend == 120 ? 1.0f : 4.0f));
                DWORD expected = 0;
                for (int shift : {0, 8, 16})
                    expected |= static_cast<DWORD>(std::lround(((dyeColor >> shift) & 255) * intensity)) << shift;
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
        }
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
    Require(device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX2));
    Require(device->SetRenderState(D3DRS_LIGHTING, FALSE));
    Require(device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
    Require(device->SetRenderState(D3DRS_ZENABLE, FALSE));
    Require(device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
    Require(device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT));
    Require(device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
    DeviceStates states{device.Get()};
    int checks = 0;
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
    std::printf("DyeTextureStagesDeviceTests: %d pixel checks PASS; real armor, tint and shader restoration verified\n", checks);
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
