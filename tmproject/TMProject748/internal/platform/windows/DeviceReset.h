#pragma once

namespace device_reset {
// Direct3D 9 Reset fails while any DEFAULT-pool object or swap-chain reference
// remains alive. MANAGED and SYSTEMMEM resources survive Reset; releasing them
// on device loss only invalidates borrowed pointers, so recovery never does.
enum class Pool { Default, Managed, SystemMemory };

constexpr bool ReleaseBeforeReset(Pool pool) { return pool == Pool::Default; }

template<class Resource>
void Release(Resource*& resource)
{
    if (resource) {
        resource->Release();
        resource = nullptr;
    }
}

// A level surface holds a reference to its render-target texture: release the
// surface first, then the texture. Both pointers end null.
template<class Texture, class Surface>
void ReleaseTarget(Texture*& texture, Surface*& surface)
{
    Release(surface);
    Release(texture);
}

template<class Texture, class Surface, unsigned Count>
void ReleaseTargets(Texture* (&textures)[Count], Surface* (&surfaces)[Count])
{
    for (unsigned i = 0; i < Count; ++i) ReleaseTarget(textures[i], surfaces[i]);
}
}
