#pragma once

namespace beriel_aura {

// BerialGuard (class 67) comes from the KR client, not from 7.48. Its later
// client effect switched the whole scene to dungeon light without weather for
// anyone who had the mob loaded, and never switched it back. The aura now only
// applies while the local player is this close (map cells) to a guard, and is
// released when no guard has refreshed it for kHoldMs.
constexpr float kRadius = 8.0f;
constexpr unsigned int kHoldMs = 500;

inline bool InRange(float dx, float dy)
{
    return dx * dx + dy * dy <= kRadius * kRadius;
}

inline bool Expired(unsigned int lastRefresh, unsigned int now)
{
    return lastRefresh != 0 && now - lastRefresh > kHoldMs;
}

// The weather the server last set is kept apart from the effective weather the
// aura overrides, so weather received during the aura is the one restored.
struct WeatherOverride
{
    int server = 0;
    unsigned int refresh = 0;

    void SetServer(int weather) { server = weather; }
    // Any guard in range refreshes the same override; 0 is reserved for "off".
    void Refresh(unsigned int now) { refresh = now ? now : 1; }
    bool Active() const { return refresh != 0; }
    int Effective() const { return Active() ? 0 : server; }
    // True once, when the hold time passes without a refresh.
    bool Release(unsigned int now)
    {
        if (!Expired(refresh, now))
            return false;
        refresh = 0;
        return true;
    }
};

} // namespace beriel_aura
