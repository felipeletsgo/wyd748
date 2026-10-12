#pragma once

#include <cstdint>

namespace instance_timer
{
    // Server milliseconds wrap modulo 2^32. Durations are signed wire seconds;
    // non-positive values stop the HUD rather than displaying a negative time.
    constexpr int Remaining(int seconds, std::uint32_t start, std::uint32_t now)
    {
        if (seconds <= 0)
            return 0;
        const auto elapsed = (now - start) / 1000u;
        return elapsed >= static_cast<std::uint32_t>(seconds)
            ? 0 : seconds - static_cast<int>(elapsed);
    }
}
