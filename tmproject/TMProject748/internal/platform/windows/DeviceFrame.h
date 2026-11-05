#pragma once
#include <d3d9.h>

namespace device_frame {
inline bool Succeeded(HRESULT result, bool& lost)
{
    if (result == D3DERR_DEVICELOST || result == D3DERR_DEVICENOTRESET) lost = true;
    return SUCCEEDED(result);
}

// Recovery state that persists across frames. The client cannot recreate its
// device, so a terminal state requests an orderly exit instead of a frozen frame.
struct Recovery {
    DWORD firstFailureMs = 0;
    unsigned failures = 0;
    HRESULT lastFailure = S_OK;
    bool fatal = false;
};

// A restore attempt on a usable device must fail this often and for this long
// before recovery becomes terminal. Renewed device loss never counts.
constexpr unsigned MaxRestoreFailures = 3;
constexpr DWORD RestoreFailureWindowMs = 10000;

inline void NoteRestore(Recovery& recovery, HRESULT result, DWORD nowMs)
{
    if (SUCCEEDED(result)) {
        recovery.failures = 0;
        return;
    }
    if (result == D3DERR_DEVICELOST) return;
    recovery.lastFailure = result;
    if (recovery.failures++ == 0) recovery.firstFailureMs = nowMs;
    if (recovery.failures >= MaxRestoreFailures &&
        nowMs - recovery.firstFailureMs >= RestoreFailureWindowMs)
        recovery.fatal = true;
}

// Only loss and pending reset are recoverable through Reset. Any other
// cooperative-level failure (for example D3DERR_DRIVERINTERNALERROR) requires
// device recreation, which this client does not support.
inline bool RecoverableStatus(HRESULT status)
{
    return SUCCEEDED(status) || status == D3DERR_DEVICELOST || status == D3DERR_DEVICENOTRESET;
}

template<class Device, class PrepareReset, class Reset, class Clear, class Clock>
bool Begin(Device* device, bool& lost, bool restored, Recovery& recovery,
    PrepareReset prepare, Reset reset, Clear clear, Clock clock)
{
    if (!device || recovery.fatal) return false;
    if (lost) {
        const HRESULT status = device->TestCooperativeLevel();
        if (status == D3DERR_DEVICELOST) return false;
        if (!RecoverableStatus(status)) {
            recovery.lastFailure = status;
            recovery.fatal = true;
            return false;
        }
        if (status == D3DERR_DEVICENOTRESET || !restored) {
            HRESULT result = prepare();
            if (SUCCEEDED(result)) {
                result = reset();
                lost = FAILED(result);
            }
            NoteRestore(recovery, result, clock());
            // Recovery owns this attempt. Drawing resumes on the next frame.
            return false;
        }
        lost = false;
    }
    if (!Succeeded(clear(), lost)) return false;
    return Succeeded(device->BeginScene(), lost);
}

template<class Device, class PrepareReset, class Reset, class Clear>
bool Begin(Device* device, bool& lost, bool restored, PrepareReset prepare, Reset reset, Clear clear)
{
    Recovery recovery;
    return Begin(device, lost, restored, recovery, prepare, reset, clear, [] { return DWORD{0}; });
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
