#include "../internal/platform/windows/DeviceBackBuffer.h"
#include <cstdio>

namespace {
struct Surface {
    HRESULT result = S_OK;
    int descriptions = 0;
    int releases = 0;
    D3DSURFACE_DESC description{};
    HRESULT GetDesc(D3DSURFACE_DESC* output) {
        ++descriptions;
        *output = description; // Simulate a partial output even when the API fails.
        return result;
    }
    ULONG Release() { ++releases; return 0; }
};
struct Device {
    HRESULT result = S_OK;
    Surface* surface = nullptr;
    int calls = 0;
    bool contract = true;
    HRESULT GetBackBuffer(UINT chain, UINT index, D3DBACKBUFFER_TYPE type, Surface** output) {
        ++calls;
        contract = contract && chain == 0 && index == 0 && type == D3DBACKBUFFER_TYPE_MONO && !*output;
        *output = surface;
        return result;
    }
};
struct Factory {
    int references = 1;
    int additions = 0;
    int releases = 0;
    bool destroyed = false;
    ULONG AddRef() { ++additions; return static_cast<ULONG>(++references); }
    ULONG Release() {
        ++releases;
        if (--references == 0) destroyed = true;
        return static_cast<ULONG>(references);
    }
};
bool Equal(const D3DSURFACE_DESC& left, const D3DSURFACE_DESC& right) {
    return left.Format == right.Format && left.Type == right.Type && left.Usage == right.Usage &&
        left.Pool == right.Pool && left.MultiSampleType == right.MultiSampleType &&
        left.MultiSampleQuality == right.MultiSampleQuality && left.Width == right.Width &&
        left.Height == right.Height;
}
}

int RunDeviceBackBufferTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) { ++failures; std::fprintf(stderr, "DeviceBackBuffer: %s\n", name); }
    };
    const HRESULT results[] = {S_OK, S_FALSE, E_FAIL, E_OUTOFMEMORY, D3DERR_DEVICELOST};
    const D3DSURFACE_DESC oldDescription{D3DFMT_R5G6B5, D3DRTYPE_SURFACE, 7, D3DPOOL_SYSTEMMEM,
        D3DMULTISAMPLE_2_SAMPLES, 3, 640, 480};
    const D3DSURFACE_DESC newDescription{D3DFMT_A8R8G8B8, D3DRTYPE_SURFACE, D3DUSAGE_RENDERTARGET,
        D3DPOOL_DEFAULT, D3DMULTISAMPLE_NONE, 0, 1280, 720};
    for (const HRESULT acquisition : results) {
        for (int output = 0; output < 2; ++output) {
            for (const HRESULT description : results) {
                Surface surface;
                surface.result = description;
                surface.description = newDescription;
                Device device{acquisition, output ? &surface : nullptr};
                auto destination = oldDescription;
                const HRESULT result = device_back_buffer::Describe<Surface>(&device, destination);
                const bool acquired = SUCCEEDED(acquisition) && output;
                const HRESULT expected = FAILED(acquisition) ? acquisition : !output ? E_FAIL : description;
                check(result == expected, "description preserves the exact failing HRESULT");
                check(Equal(destination, acquired && SUCCEEDED(description) ? newDescription : oldDescription),
                    "failed queries cannot publish partial or zeroed descriptors");
                check(surface.descriptions == (acquired ? 1 : 0), "no surface access after failed acquisition");
                check(surface.releases == output, "release every non-null output exactly once, including failed HRESULTs");
                check(device.calls == 1 && device.contract, "first mono back-buffer with initialized output");
                // A later reset/query can recover without depending on the failed output.
                surface.result = device.result = S_OK;
                device.surface = &surface;
                check(device_back_buffer::Describe<Surface>(&device, destination) == S_OK &&
                    Equal(destination, newDescription), "successful retry replaces the cached descriptor");
                check(surface.releases == output + 1 && device.contract, "retry releases its new reference");
            }
            for (const HRESULT saveResult : results) {
                Surface surface;
                Device device{acquisition, output ? &surface : nullptr};
                int saves = 0;
                bool correctSurface = true;
                const HRESULT result = device_back_buffer::Capture<Surface>(&device, [&](Surface* received) {
                    ++saves;
                    correctSurface = received == &surface;
                    return saveResult;
                });
                const bool acquired = SUCCEEDED(acquisition) && output;
                check(result == (FAILED(acquisition) ? acquisition : !output ? E_FAIL : saveResult),
                    "capture propagates acquisition or encoder failure");
                check(saves == (acquired ? 1 : 0) && correctSurface, "encoder only receives an acquired surface");
                check(surface.releases == output && surface.descriptions == 0, "capture releases without a description query");
                check(device.calls == 1 && device.contract, "capture keeps the mono acquisition contract");
            }
        }
    }
    auto destination = oldDescription;
    check(device_back_buffer::Describe<Surface>(static_cast<Device*>(nullptr), destination) == E_POINTER &&
        Equal(destination, oldDescription), "missing device preserves descriptor");
    int saves = 0;
    check(device_back_buffer::Capture<Surface>(static_cast<Device*>(nullptr), [&](Surface*) {
        ++saves; return S_OK;
    }) == E_POINTER && saves == 0, "missing capture device never invokes the encoder");
    Surface surface;
    Device device{S_OK, &surface};
    {
        device_back_buffer::OwnedSurface<Surface> owned;
        check(device_back_buffer::Acquire(&device, owned) == S_OK, "owned acquisition succeeds");
        check(device_back_buffer::Acquire(&device, owned) == E_UNEXPECTED && device.calls == 1,
            "reused owner cannot overwrite and leak an existing reference");
    }
    check(surface.releases == 1, "owner releases once at scope exit");
    Factory factory;
    Factory* published = &factory;
    for (int attempt = 1; attempt <= 3; ++attempt) {
        int cleanups = 0;
        device_back_buffer::CleanupForRetry(published, [&] {
            ++cleanups;
            published->Release();
            published = nullptr;
        });
        check(cleanups == 1 && published == &factory, "retry cleanup restores the factory pointer");
        check(factory.references == 1 && !factory.destroyed && factory.additions == attempt &&
            factory.releases == attempt, "factory remains alive with exactly one owned reference");
    }
    published->Release();
    published = nullptr;
    check(factory.references == 0 && factory.destroyed && factory.releases == 4,
        "final teardown releases the retained factory without leaking");
    int cleanups = 0;
    device_back_buffer::CleanupForRetry(published, [&] { ++cleanups; });
    check(!published && cleanups == 1, "missing factory still permits final device cleanup");
    return failures;
}
