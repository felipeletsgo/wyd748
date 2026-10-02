#pragma once

// Local HP projection applied to an already resolved attack target.
// The server remains authoritative; this only reproduces the client's existing
// display arithmetic. No entity lookup, UI, party, request or effect work.
//
// Every conversion below is the one the original expressions performed
// implicitly on unsigned HP fields: signed operands become unsigned, an
// unsigned result assigned to int wraps, and big-pool values pass through
// short before widening back to the unsigned HP field.
namespace attack_target
{
// Healing (instance type 6) on a regular HP pool, first receive loop.
inline void HealPool(unsigned int& curHp, unsigned int maxHp, int damage, int damageRate)
{
    const int heal = damage / damageRate;
    int hp = static_cast<int>(curHp - static_cast<unsigned int>(heal));
    if (static_cast<unsigned int>(hp) > maxHp)
        hp = static_cast<int>(maxHp);
    if (hp <= 0)
        curHp = 0;
    else
        curHp = static_cast<unsigned int>(hp);
}

// Healing on a large ("big") HP pool. A zero rate is normalized in place,
// because the caller reuses the same rate after this call.
inline void HealBigPool(unsigned int& curHp, unsigned int& bigHp, int damage, int& damageRate)
{
    if (bigHp == static_cast<unsigned int>(damage))
        bigHp = 0;
    else
        bigHp -= static_cast<unsigned int>(damage);
    if (!damageRate)
        damageRate = 1;
    curHp = static_cast<unsigned int>(static_cast<short>(bigHp / static_cast<unsigned int>(damageRate)));
}

// Damage, first receive loop. Returns the value the caller subtracts from its
// requested-HP snapshot: zero for implausible damage, divided for regular pools.
inline int Damage(unsigned int& curHp, unsigned int& bigHp, bool bigPool, int damage, int damageRate)
{
    int value = damage;
    if (value > 1000000)
        value = 0;
    if (value >= 0) {
        if (!bigPool) {
            value /= damageRate;
            int hp = static_cast<int>(curHp - static_cast<unsigned int>(value));
            if (hp < 0)
                hp = 0;
            curHp = static_cast<unsigned int>(hp);
        } else {
            bigHp -= static_cast<unsigned int>(value);
            curHp = static_cast<unsigned int>(static_cast<short>(bigHp / static_cast<unsigned int>(damageRate)));
        }
    }
    return value;
}

// Second receive loop, regular pool, for both healing and damage. The original
// "<= 0" test on an unsigned difference only matches an exact zero.
inline void SubtractPool(unsigned int& curHp, int damage, int damageRate)
{
    const unsigned int delta = static_cast<unsigned int>(damage / damageRate);
    if (curHp - delta == 0u)
        curHp = 0;
    else
        curHp -= delta;
}

// Second receive loop, big pool: the HP field takes the truncated pool value.
inline void SubtractBigPool(unsigned int& curHp, unsigned int& bigHp, int damage)
{
    if (bigHp == static_cast<unsigned int>(damage))
        bigHp = 0;
    else
        bigHp -= static_cast<unsigned int>(damage);
    curHp = static_cast<unsigned int>(static_cast<short>(bigHp));
}
}
