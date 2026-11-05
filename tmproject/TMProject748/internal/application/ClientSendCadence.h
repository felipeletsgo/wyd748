#pragma once

namespace client_send_cadence
{
// Preserve the legacy action (>1000 ms) and attack (>=1000 ms) boundaries.
// Subtraction keeps both gates valid when the 32-bit clock wraps.
constexpr bool Allows(unsigned int now, unsigned int last, bool action,
    bool lastAction, bool attack, bool lastAttack)
{
    const unsigned int elapsed = now - last;
    return (!action || !lastAction || elapsed > 1000) &&
        (!attack || !lastAttack || elapsed >= 1000);
}
}
