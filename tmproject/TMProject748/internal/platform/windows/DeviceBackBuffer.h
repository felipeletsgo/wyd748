#pragma once
#include <d3d9.h>

namespace device_back_buffer {
template<class Surface>
class OwnedSurface {
public:
    Surface* value = nullptr;
    OwnedSurface() = default;
    OwnedSurface(const OwnedSurface&) = delete;
    OwnedSurface& operator=(const OwnedSurface&) = delete;
    ~OwnedSurface() { if (value) value->Release(); }
};

template<class Device, class Surface>
HRESULT Acquire(Device* device, OwnedSurface<Surface>& surface)
{
    if (!device) return E_POINTER;
    if (surface.value) return E_UNEXPECTED;
    const HRESULT result = device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &surface.value);
    if (FAILED(result)) return result;
    return surface.value ? result : E_FAIL;
}

template<class Surface = IDirect3DSurface9, class Device>
HRESULT Describe(Device* device, D3DSURFACE_DESC& destination)
{
    OwnedSurface<Surface> surface;
    HRESULT result = Acquire(device, surface);
    if (FAILED(result)) return result;
    D3DSURFACE_DESC pending{};
    result = surface.value->GetDesc(&pending);
    if (SUCCEEDED(result)) destination = pending;
    return result;
}

template<class Surface = IDirect3DSurface9, class Device, class Save>
HRESULT Capture(Device* device, Save save)
{
    OwnedSurface<Surface> surface;
    const HRESULT result = Acquire(device, surface);
    if (FAILED(result)) return result;
    return save(surface.value);
}

// Full teardown releases the factory too; retain its reference for recreation.
template<class Factory, class Cleanup>
void CleanupForRetry(Factory*& factory, Cleanup cleanup)
{
    Factory* retained = factory;
    if (retained) retained->AddRef();
    cleanup();
    factory = retained;
}
}
