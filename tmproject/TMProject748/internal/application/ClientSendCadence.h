#pragma once

namespace client_send_cadence
{
constexpr unsigned int LegacyActionInterval = 1000;
constexpr unsigned int GroundActionInterval = 100;

// Ground clicks can replace a short route before it ends. Other callers use the legacy interval.
// Subtraction keeps both gates valid when the 32-bit clock wraps.
constexpr bool Allows(unsigned int now, unsigned int last, bool action,
    bool lastAction, bool attack, bool lastAttack, unsigned int actionInterval = LegacyActionInterval)
{
    const unsigned int elapsed = now - last;
    return (!action || !lastAction || elapsed > actionInterval) &&
        (!attack || !lastAttack || elapsed >= 1000);
}
}
