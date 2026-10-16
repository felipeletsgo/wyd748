#pragma once

#include <d3d9.h>

namespace native_inventory_lighting {
// Native 7.48 block 3 is mapped to 4 because this port already uses 3 for 2D setup.
constexpr int RenderBlock = 4;
constexpr DWORD Ambient = 0xFFFFFFFF;

inline void Prepare(D3DLIGHT9 (&lights)[2])
{
    // FUN_0042dea5 changes RGB only, preserving each light's other fields.
    for (int index = 0; index < 2; ++index) {
        const float intensity = index == 0 ? 1.0f : 0.6f;
        lights[index].Diffuse.r = lights[index].Diffuse.g = lights[index].Diffuse.b = intensity;
        lights[index].Specular.r = lights[index].Specular.g = lights[index].Specular.b = intensity;
    }
}

constexpr DWORD AlphaReference(DWORD bitCount)
{
    return bitCount == 32 ? 0xAA : 0xFF;
}
}
