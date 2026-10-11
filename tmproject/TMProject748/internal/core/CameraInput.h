#pragma once

#include <windows.h>
#include <dinput.h>

// A lost foreground device may stay unavailable for many frames. Try acquiring
// once per frame, never spin or expose a previous/partially written sample.
// Device is a template parameter so the actual polling path can use a fake in tests.
template <typename Device>
bool PollCameraMouse(Device* device, DIMOUSESTATE2& state)
{
    state = {};
    if (!device)
        return false;
    if (FAILED(device->GetDeviceState(sizeof(state), &state)))
    {
        state = {};
        device->Acquire();
        return false;
    }
    return true;
}

inline float CameraZoomDistance(float distance, float nearLimit, float farLimit,
    int cameraView, int wheel, int viewChange)
{
    const int direction = cameraView == 0 ? viewChange : wheel;
    if (distance > nearLimit && direction < 0)
        distance += static_cast<float>(direction) / 240.0f;
    if (distance < nearLimit)
        distance = nearLimit;
    if (direction > 0 && farLimit > distance)
        distance += static_cast<float>(direction) / 240.0f;
    if (distance > farLimit)
        distance = farLimit;
    return distance;
}
