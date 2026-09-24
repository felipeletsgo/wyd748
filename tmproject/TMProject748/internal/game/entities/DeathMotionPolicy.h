#pragma once

namespace death_motion {

// An unfinished route must not replace a death animation with travel motion.
constexpr bool MayEnterTravelAnimation(int currentHp, bool dead, bool sliding)
{
    return currentHp > 0 && !dead && !sliding;
}

constexpr bool ShouldEnterDeath(int currentHp, bool dead)
{
    return currentHp <= 0 && !dead;
}

constexpr bool CanOfferRespawnPrompt(int currentHp, bool dead, bool hasFamiliar,
    bool inTown, bool inCastleTown, bool promptVisible)
{
    return (currentHp <= 0 || dead) && !hasFamiliar &&
        (!inTown || inCastleTown) && !promptVisible;
}

// The compact field input path must preserve the native dead-player click
// fallback even though it rejects ordinary world actions at zero HP.
constexpr bool ShouldOpenRespawnPrompt(int currentHp, bool dead, bool leftButtonDown,
    bool hasFamiliar, bool inTown, bool inCastleTown, bool promptVisible)
{
    return leftButtonDown && CanOfferRespawnPrompt(currentHp, dead, hasFamiliar,
        inTown, inCastleTown, promptVisible);
}

// If a mesh never completes its death animation, offer the same prompt after
// a bounded delay instead of leaving the player waiting for a click forever.
constexpr bool ShouldOfferTimedRespawnPrompt(unsigned int deathTime, unsigned int now,
    int currentHp, bool dead, bool hasFamiliar, bool inTown, bool inCastleTown,
    bool promptVisible)
{
    return deathTime != 0 && now - deathTime >= 3000u &&
        CanOfferRespawnPrompt(currentHp, dead, hasFamiliar, inTown,
            inCastleTown, promptVisible);
}

} // namespace death_motion
