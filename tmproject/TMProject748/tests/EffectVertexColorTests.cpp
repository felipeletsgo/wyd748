#include "../internal/render/mesh/EffectVertexColor.h"
#include "../internal/render/mesh/DyeTextureStages.h"
#include <cstdio>
#include <cstring>
#include <initializer_list>
#include <cmath>

namespace
{
struct Vertex { float position[3]; unsigned int diffuse; float uv[2]; };
struct DyeDevice
{
    DWORD stages[2][33]{};
    void SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE state, DWORD value)
    {
        stages[stage][state] = value;
    }
};
// Grayscale stage-0 fixtures replicate each argument across all three channels.
// DOTPRODUCT3 expands unsigned inputs to signed [-1, 1] before the dot product.
float DyeStage(const DyeDevice& device, unsigned int stage, float texture, float current)
{
    float value = 0;
    switch (device.stages[stage][D3DTSS_COLOROP])
    {
    case D3DTOP_MODULATE: value = texture * current; break;
    case D3DTOP_MODULATE4X: value = 4 * texture * current; break;
    case D3DTOP_DOTPRODUCT3: value = 12 * (texture - 0.5f) * (current - 0.5f); break;
    case D3DTOP_ADD: value = texture + current; break;
    default: return -1;
    }
    return value < 0 ? 0 : (value > 1 ? 1 : value);
}
struct Descriptor { unsigned int Size = 0; unsigned int FVF = 0; };
struct Buffer
{
    Vertex vertices[2]{{{1, 2, 3}, 7, {4, 5}}, {{6, 7, 8}, 9, {10, 11}}};
    Descriptor desc{sizeof(vertices), 322};
    int getResult = 0, lockResult = 0, unlockResult = 0;
    int gets = 0, locks = 0, unlocks = 0;
    bool nullData = false;
    int GetDesc(Descriptor* out) { ++gets; *out = desc; return getResult; }
    int Lock(unsigned int, unsigned int, void** out, unsigned int)
    {
        ++locks;
        *out = nullData ? nullptr : vertices;
        return lockResult;
    }
    int Unlock() { ++unlocks; return unlockResult; }
};
bool Paint(Buffer* buffer, unsigned int stride = sizeof(Vertex))
{
    return effect_vertex_color::TrySetDiffuse<Vertex, Descriptor>(buffer, stride, 322, 0xFFAAAAAA);
}
}

int RunEffectVertexColorTests(int& checks)
{
    int failures = 0;
    auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) { ++failures; std::printf("FAIL EffectVertexColor: %s\n", name); }
    };
    for (short legend = 116; legend <= 125; ++legend)
    {
        // Native adapter branches: common, Voodoo/Intel/G400, TNT only.
        for (int adapter = 0; adapter < 3; ++adapter)
        {
            const bool legacy = adapter == 1;
            const bool tnt = adapter == 2;
            for (char alpha : {'A', 'C'})
            {
                DyeDevice device;
                device.stages[0][D3DTSS_COLORARG2] = D3DTA_TFACTOR;
                device.stages[1][D3DTSS_TEXCOORDINDEX] = 0;
                dye_texture_stages::Apply(device, legend, alpha, legacy, tnt);
                check(device.stages[0][D3DTSS_COLORARG2] == D3DTA_CURRENT,
                    "dye does not inherit warm texture factor");
                check(device.stages[1][D3DTSS_TEXCOORDINDEX] == 1,
                    "dye uses animated second UV channel");
                // Literal operands from FUN_004c3eec, independent of enum names.
                check(device.stages[0][D3DTSS_COLOROP] == (adapter != 0 ? 4u : 0x18u),
                    "native dye stage 0 numeric operation");
                check(device.stages[1][D3DTSS_COLOROP] == (adapter != 0 ? 7u : (legend == 120 ? 4u : 6u)),
                    "native dye stage 1 numeric operation");
                check(device.stages[1][D3DTSS_ALPHAOP] ==
                    (alpha == 'C' && !legacy ? 0u : D3DTOP_DISABLE),
                    "native opaque and alpha material handling");
            }
        }
    }
    for (float lighting : {0.55f, 1.0f})
    {
        DyeDevice blue, black;
        dye_texture_stages::Apply(blue, 116, 'C', false);
        dye_texture_stages::Apply(black, 120, 'C', false);
        for (float base : {0.25f, 0.5f, 0.75f, 1.0f})
        {
            const float density = DyeStage(blue, 0, base, lighting);
            // Actual bl0000.wys texel (24, 82, 186), not a guessed blue constant.
            const float red = DyeStage(blue, 1, 24.0f / 255, density);
            const float green = DyeStage(blue, 1, 82.0f / 255, density);
            const float blueChannel = DyeStage(blue, 1, 186.0f / 255, density);
            check(base > 0.5f ? blueChannel > red && green > red : blueChannel == 0,
                "blue +1 retains chroma and does not add a gray illumination floor");
            check(red <= 96.0f / 255, "blue +1 cannot become white under selection lighting");
            check(std::fabs(DyeStage(black, 1, 95.0f / 255,
                DyeStage(black, 0, base, lighting)) - density * 95.0f / 255) < 0.00001f,
                "black uses the same native density without the fourfold color gain");
        }
    }
    check(!Paint(nullptr), "null vertex buffer from crash is rejected");
    Buffer valid;
    Buffer original;
    check(Paint(&valid), "loaded buffer accepts color update");
    check(valid.gets == 1 && valid.locks == 1 && valid.unlocks == 1,
        "successful update balances lock exactly once");
    for (int i = 0; i < 2; ++i)
    {
        check(valid.vertices[i].diffuse == 0xFFAAAAAA, "all vertex colors updated");
        check(std::memcmp(valid.vertices[i].position, original.vertices[i].position,
            sizeof(Vertex::position)) == 0 &&
            std::memcmp(valid.vertices[i].uv, original.vertices[i].uv, sizeof(Vertex::uv)) == 0,
            "geometry and UVs preserved");
    }
    for (int fault = 0; fault < 8; ++fault)
    {
        Buffer buffer;
        unsigned int stride = sizeof(Vertex);
        switch (fault)
        {
        case 0: buffer.getResult = -1; break;
        case 1: buffer.desc.Size = 0; break;
        case 2: ++buffer.desc.Size; break;
        case 3: buffer.desc.FVF = 0; break;
        case 4: ++stride; break;
        case 5: buffer.lockResult = -1; break;
        case 6: buffer.nullData = true; break;
        case 7: buffer.unlockResult = -1; break;
        }
        check(!Paint(&buffer, stride), "device or layout failure reported");
        check(buffer.locks == (fault >= 5 ? 1 : 0), "invalid layout never locks");
        check(buffer.unlocks == (fault >= 6 ? 1 : 0), "only successful locks are unlocked");
        if (fault != 7)
            check(std::memcmp(buffer.vertices, original.vertices, sizeof(buffer.vertices)) == 0,
                "failure before write leaves buffer unchanged");
    }
    return failures;
}
