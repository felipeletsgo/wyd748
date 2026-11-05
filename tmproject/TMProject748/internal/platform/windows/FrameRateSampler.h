#pragma once
#include <windows.h>

namespace frame_rate {
constexpr DWORD SampleIntervalMs = 2000;

// Counts presented frames and publishes a rate after each interval. Unsigned
// millisecond differences stay valid across the 49.7-day timeGetTime wrap.
class Sampler {
public:
    // Returns true and writes framesPerSecond when an interval completes.
    bool Frame(DWORD nowMs, float& framesPerSecond)
    {
        ++frames_;
        if (!started_) {
            started_ = true;
            lastMs_ = nowMs;
            return false;
        }
        const DWORD elapsed = nowMs - lastMs_;
        if (elapsed <= SampleIntervalMs) return false;
        framesPerSecond = static_cast<float>(frames_) * 1000.0f / static_cast<float>(elapsed);
        lastMs_ = nowMs;
        frames_ = 0;
        return true;
    }

private:
    DWORD lastMs_ = 0;
    DWORD frames_ = 0;
    bool started_ = false;
};
}
