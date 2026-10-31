#pragma once

namespace apprentice_mantle_tint_748 {
constexpr bool IsCitizenApprenticeMantle(int boneAnimation, int skin)
{
    return boneAnimation == 85 && skin >= 40 && skin <= 49;
}

// The citizen textures contain the emblem but use a pale cloth base. Tint the
// material green. Multiply the computed light and status colors, including
// zero emission, rather than replacing them with an unlit fixed material.
template<class Material>
inline void ApplyGreen(Material& material)
{
    material.Diffuse.r *= 0.14f;
    material.Diffuse.g *= 0.42f;
    material.Diffuse.b *= 0.22f;
    material.Specular.r *= 0.14f;
    material.Specular.g *= 0.42f;
    material.Specular.b *= 0.22f;
    material.Emissive.r *= 0.14f;
    material.Emissive.g *= 0.42f;
    material.Emissive.b *= 0.22f;
}
}
