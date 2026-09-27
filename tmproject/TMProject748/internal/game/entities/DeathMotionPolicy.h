#pragma once

namespace death_motion {

// Automatic offers are one-shot per death. Dismissing the dialog allows only
// a new player action to reopen it; a recall channel suppresses both sources.
constexpr bool MayOfferRespawnPrompt(bool alreadyOffered, bool playerAction, bool channeling)
{
    return !channeling && (playerAction || !alreadyOffered);
}

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

// The native field tick recalls a character still dead after three minutes.
constexpr bool ShouldAutoRecallDeadPlayer(unsigned int deathTime, unsigned int now,
    bool dead, int worldX, int worldY)
{
    return deathTime != 0 && now - deathTime > 180000u && dead &&
        (worldX >> 7) != 1 && (worldY >> 7) != 1;
}

// Once the five-second recall request has been sent, do not replay its portal
// effect while waiting for the timer to be cleared on the following tick.
constexpr bool ShouldAdvanceRespawnRecallCountdown(unsigned int startTime,
    unsigned int now, bool requestPending)
{
    return startTime != 0 && requestPending && now - startTime <= 5000u;
}

constexpr unsigned int RespawnRecallSecondsRemaining(unsigned int startTime,
    unsigned int now)
{
    return (5000u - (now - startTime)) / 1000u;
}

} // namespace death_motion
