#pragma once

// Local HP projection applied to an already resolved attack target.
// The server remains authoritative. Projection uses signed intermediates and
// saturates at the pool boundaries instead of wrapping unsigned HP fields.
namespace attack_target
{
inline int SafeDamageRate(int rate) { return rate > 0 ? rate : 1; }

// An authoritative uint32 snapshot supersedes local attack projection. Keep
// an already active large pool in sync so the next draw/hit cannot restore it.
inline void SyncActivePool(unsigned int current, unsigned int maximum,
    unsigned int& bigCurrent, unsigned int& bigMaximum)
{
    if (bigMaximum) {
        bigCurrent = current;
        bigMaximum = maximum;
    }
}

inline unsigned int ProjectPool(unsigned int hp, long long damage, unsigned int maximum = 0xFFFFFFFFu)
{
    const long long next = static_cast<long long>(hp) - damage;
    return next <= 0 ? 0u : next >= maximum ? maximum : static_cast<unsigned int>(next);
}

// Healing (instance type 6) on a regular HP pool, first receive loop.
inline void HealPool(unsigned int& curHp, unsigned int maxHp, int damage, int damageRate)
{
    curHp = ProjectPool(curHp, damage / SafeDamageRate(damageRate), maxHp);
}

// Healing on a large ("big") HP pool. An invalid rate is normalized in place,
// because the caller reuses the same rate after this call.
inline void HealBigPool(unsigned int& curHp, unsigned int& bigHp, int damage, int& damageRate)
{
    bigHp = ProjectPool(bigHp, damage);
    damageRate = SafeDamageRate(damageRate);
    curHp = bigHp / static_cast<unsigned int>(damageRate);
}

// Damage, first receive loop. Returns the value the caller subtracts from its
// requested-HP snapshot: divided for regular pools. Wide values have already
// passed the receive contract's validation and must not be discarded.
inline int Damage(unsigned int& curHp, unsigned int& bigHp, bool bigPool, int damage, int damageRate)
{
    int value = damage;
    if (value >= 0) {
        if (!bigPool) {
            value /= SafeDamageRate(damageRate);
            curHp = ProjectPool(curHp, value);
        } else {
            bigHp = ProjectPool(bigHp, value);
            curHp = bigHp / static_cast<unsigned int>(SafeDamageRate(damageRate));
        }
    }
    return value;
}

// Apply the same signed HP delta to the local request snapshot that the
// target-state helpers apply to the displayed HP. A negative delta is damage;
// a positive delta is healing.
inline void ApplyPendingHPDelta(unsigned int& pendingHp, int delta, unsigned int maxHp)
{
    const long long next = static_cast<long long>(pendingHp) + delta;
    if (next <= 0)
        pendingHp = 0;
    else if (next >= static_cast<long long>(maxHp))
        pendingHp = maxHp;
    else
        pendingHp = static_cast<unsigned int>(next);
}

// Subtract projected damage without unsigned underflow.
inline void SubtractPendingHP(unsigned int& pendingHp, int damage)
{
    if (damage <= 0)
        return;
    const unsigned int delta = static_cast<unsigned int>(damage);
    pendingHp = delta >= pendingHp ? 0u : pendingHp - delta;
}

// Second receive loop, regular pool, for both healing and damage.
inline void SubtractPool(unsigned int& curHp, int damage, int damageRate)
{
    curHp = ProjectPool(curHp, damage / SafeDamageRate(damageRate));
}

// Second receive loop, big pool: the HP field takes the 32-bit pool value.
inline void SubtractBigPool(unsigned int& curHp, unsigned int& bigHp, int damage)
{
    bigHp = ProjectPool(bigHp, damage);
    curHp = bigHp;
}
}
