#pragma once
#include <d3d9.h>

namespace device_frame {
inline bool Succeeded(HRESULT result, bool& lost)
{
    if (result == D3DERR_DEVICELOST || result == D3DERR_DEVICENOTRESET) lost = true;
    return SUCCEEDED(result);
}

template<class Device, class PrepareReset, class Reset, class Clear>
bool Begin(Device* device, bool& lost, bool restored, PrepareReset prepare, Reset reset, Clear clear)
{
    if (!device) return false;
    if (lost) {
        const HRESULT status = device->TestCooperativeLevel();
        if (status == D3DERR_DEVICELOST) return false;
        if (FAILED(status) && status != D3DERR_DEVICENOTRESET) return false;
        if (status == D3DERR_DEVICENOTRESET || !restored) {
            if (FAILED(prepare())) return false;
            lost = FAILED(reset());
            // Recovery owns this attempt. Drawing resumes on the next frame.
            return false;
        }
        lost = false;
    }
    if (!Succeeded(clear(), lost)) return false;
    return Succeeded(device->BeginScene(), lost);
}

template<class Device, class Present>
bool End(Device* device, bool& lost, Present present)
{
    if (!device || !Succeeded(device->EndScene(), lost)) return false;
    return Succeeded(present(), lost);
}

// Local ownership keeps early returns from abandoning a successfully opened scene.
template<class Renderer>
class Scene {
public:
    explicit Scene(Renderer& renderer) : renderer_(renderer) {}
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
    ~Scene() { if (open_) renderer_.Unlock(0); }
    bool Begin(int clear) {
        if (open_) return false;
        open_ = renderer_.Lock(clear) == 1;
        return open_;
    }
    bool End(int present) {
        if (!open_) return false;
        open_ = false;
        return renderer_.Unlock(present) == 1;
    }
private:
    Renderer& renderer_;
    bool open_ = false;
};
}
