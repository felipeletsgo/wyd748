#include "../internal/application/AttackTargetState.h"

#include <cstdio>
#include <initializer_list>

namespace
{
struct Score { unsigned int MaxHP; unsigned int CurHP; };
struct Target { Score m_stScore; unsigned int m_BigHp; unsigned int m_MaxBigHp; };
struct Dam { short Damage; };

// Legacy statements are compared only on their non-overflow domain.
#pragma warning(push)
#pragma warning(disable: 4018 4245 4389 4456)
void LegacyHeal(Target* pTargetHuman, const Dam& dam, int& nDamageRate)
{
    if (!pTargetHuman->m_MaxBigHp)
    {
        int HealDam = dam.Damage / nDamageRate;
        int Dam = pTargetHuman->m_stScore.CurHP - HealDam;

        if (Dam > pTargetHuman->m_stScore.MaxHP)
            Dam = pTargetHuman->m_stScore.MaxHP;
        if (Dam <= 0)
            pTargetHuman->m_stScore.CurHP = 0;
        else
            pTargetHuman->m_stScore.CurHP = Dam;
    }
    else
    {
        int Dam = dam.Damage;
        if (pTargetHuman->m_BigHp == Dam)
            pTargetHuman->m_BigHp = 0;
        else
            pTargetHuman->m_BigHp -= Dam;
        if (!nDamageRate)
            nDamageRate = 1;

        pTargetHuman->m_stScore.CurHP = pTargetHuman->m_BigHp / nDamageRate;
    }
}

int LegacyDamage(Target* pTargetHuman, const Dam& dam, int nDamageRate)
{
    int nValue = dam.Damage;
    if (nValue > 1000000)
        nValue = 0;
    if (nValue >= 0)
    {
        if (!pTargetHuman->m_MaxBigHp)
        {
            nValue /= nDamageRate;
            int hp = pTargetHuman->m_stScore.CurHP - nValue;
            if (hp < 0)
                hp = 0;
            pTargetHuman->m_stScore.CurHP = hp;
        }
        else
        {
            pTargetHuman->m_BigHp -= nValue;
            pTargetHuman->m_stScore.CurHP = pTargetHuman->m_BigHp / nDamageRate;
        }
    }
    return nValue;
}

void LegacySubtract(Target* pTargetHuman, const Dam& dam, int nDamageRate)
{
    if (!pTargetHuman->m_MaxBigHp)
    {
        if (pTargetHuman->m_stScore.CurHP - dam.Damage / nDamageRate <= 0)
            pTargetHuman->m_stScore.CurHP = 0;
        else
            pTargetHuman->m_stScore.CurHP -= dam.Damage / nDamageRate;
    }
    else
    {
        if (pTargetHuman->m_BigHp == dam.Damage)
            pTargetHuman->m_BigHp = 0;
        else
            pTargetHuman->m_BigHp -= dam.Damage;
        pTargetHuman->m_stScore.CurHP = pTargetHuman->m_BigHp;
    }
}
#pragma warning(pop)

bool Same(const Target& a, const Target& b)
{
    return a.m_stScore.CurHP == b.m_stScore.CurHP && a.m_stScore.MaxHP == b.m_stScore.MaxHP &&
        a.m_BigHp == b.m_BigHp && a.m_MaxBigHp == b.m_MaxBigHp;
}
}

int RunAttackTargetStateTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool passed, const char* message) {
        ++checks;
        if (!passed) {
            ++failures;
            std::fprintf(stderr, "FAIL attack target state: %s\n", message);
        }
    };

    const unsigned int hps[]{0u, 1u, 2u, 99u, 100u, 101u, 32767u, 32768u, 65535u, 65536u,
        0x7FFFFFFFu, 0x80000000u, 0xFFFFFFFEu, 0xFFFFFFFFu};
    const unsigned int maxima[]{0u, 1u, 100u, 32768u, 0xFFFFFFFFu};
    const short damages[]{-32768, -32767, -100, -7, -6, -5, -1, 0, 1, 5, 99, 100, 101, 32767};
    const int rates[]{1, 2, 3, 127};

    for (unsigned int curHp : hps)
        for (unsigned int bigHp : hps)
            for (unsigned int bigPool : {0u, 1u})
                for (short damage : damages)
                    for (int rate : rates) {
                        const Dam dam{damage};
                        for (unsigned int maxHp : maxima) {
                            Target legacy{{maxHp, curHp}, bigHp, bigPool};
                            Target actual = legacy;
                            int legacyRate = rate;
                            int actualRate = rate;
                            LegacyHeal(&legacy, dam, legacyRate);
                            if (!actual.m_MaxBigHp)
                                attack_target::HealPool(actual.m_stScore.CurHP, actual.m_stScore.MaxHP,
                                    damage, actualRate);
                            else
                                attack_target::HealBigPool(actual.m_stScore.CurHP, actual.m_BigHp,
                                    damage, actualRate);
                            const long long projected = static_cast<long long>(bigPool ? bigHp : curHp) -
                                (bigPool ? damage : damage / rate);
                            if (projected >= 0 && projected <= 0x7FFFFFFF && maxHp <= 0x7FFFFFFFu &&
                                curHp <= 0x7FFFFFFFu)
                                check(Same(actual, legacy) && actualRate == legacyRate,
                                    "healing preserves non-overflow legacy arithmetic");
                        }

                        Target legacy{{100u, curHp}, bigHp, bigPool};
                        Target actual = legacy;
                        const int legacyValue = LegacyDamage(&legacy, dam, rate);
                        const int actualValue = attack_target::Damage(actual.m_stScore.CurHP,
                            actual.m_BigHp, actual.m_MaxBigHp != 0, damage, rate);
                        if (curHp <= 0x7FFFFFFFu && (damage < 0 || !bigPool || bigHp >= static_cast<unsigned int>(damage)))
                            check(Same(actual, legacy) && actualValue == legacyValue,
                                "first-loop damage preserves non-overflow legacy arithmetic");

                        Target legacySecond{{100u, curHp}, bigHp, bigPool};
                        Target actualSecond = legacySecond;
                        LegacySubtract(&legacySecond, dam, rate);
                        if (!actualSecond.m_MaxBigHp)
                            attack_target::SubtractPool(actualSecond.m_stScore.CurHP, damage, rate);
                        else
                            attack_target::SubtractBigPool(actualSecond.m_stScore.CurHP,
                                actualSecond.m_BigHp, damage);
                        const long long second = static_cast<long long>(bigPool ? bigHp : curHp) -
                            (bigPool ? damage : damage / rate);
                        if (second >= 0 && second <= 0xFFFFFFFFLL)
                            check(Same(actualSecond, legacySecond), "second loop preserves non-overflow legacy arithmetic");
                    }

    // Invalid rates are normalized; big-pool healing persists that correction.
    unsigned int curHp = 10u, bigHp = 50u;
    int rate = 0;
    attack_target::HealBigPool(curHp, bigHp, 10, rate);
    check(rate == 1 && bigHp == 40u && curHp == 40u, "big-pool healing normalizes a zero rate in place");
    // Big pools keep 32-bit HP instead of the native short truncation.
    curHp = 0u; bigHp = 4000001u;
    attack_target::SubtractBigPool(curHp, bigHp, 1);
    check(curHp == 4000000u, "big pool keeps 32-bit HP");
    // Overkill must clear HP instead of resurrecting the target through wrap.
    curHp = 5u;
    attack_target::SubtractPool(curHp, 10, 1);
    check(curHp == 0u, "overkill saturates at zero");

    const int wideDamages[]{-2147483647, -32768, -1, 0, 1, 32767, 1000001, 2147483647};
    for (unsigned int hp : hps)
        for (int damage : wideDamages)
            for (int divisor : {-128, -1, 0, 1, 3, 127}) {
                const int safeRate = divisor > 0 ? divisor : 1;
                const auto reference = [](unsigned int initial, long long delta, unsigned int maximum) {
                    const long long value = static_cast<long long>(initial) - delta;
                    return value < 0 ? 0u : value > maximum ? maximum : static_cast<unsigned int>(value);
                };
                curHp = hp;
                attack_target::SubtractPool(curHp, damage, divisor);
                check(curHp == reference(hp, damage / safeRate, 0xFFFFFFFFu), "wide regular projection saturates");
                curHp = hp; bigHp = hp;
                attack_target::SubtractBigPool(curHp, bigHp, damage);
                check(curHp == reference(hp, damage, 0xFFFFFFFFu) && bigHp == curHp, "wide big projection saturates");
                curHp = hp; bigHp = hp;
                const int actual = attack_target::Damage(curHp, bigHp, true, damage, divisor);
                check(actual == damage && bigHp == (damage < 0 ? hp : reference(hp, damage, 0xFFFFFFFFu)),
                    "wide first-loop damage is retained without overflow");
                check(curHp == (damage < 0 ? hp : bigHp / safeRate), "wide first-loop HP uses a safe rate");
                curHp = hp;
                attack_target::HealPool(curHp, 4000000000u, damage, divisor);
                check(curHp == reference(hp, damage / safeRate, 4000000000u), "wide healing respects maximum HP");
                curHp = hp; bigHp = hp;
                rate = divisor;
                attack_target::HealBigPool(curHp, bigHp, damage, rate);
                check(bigHp == reference(hp, damage, 0xFFFFFFFFu) && curHp == bigHp / safeRate && rate == safeRate,
                    "wide big-pool healing saturates and normalizes the rate");
            }

    for (unsigned int current : hps)
        for (unsigned int maximum : maxima) {
            curHp = 123u; bigHp = 456u;
            attack_target::SyncActivePool(current, maximum, curHp, bigHp);
            check(curHp == current && bigHp == maximum, "authoritative snapshots replace active big pools");
            curHp = 123u; bigHp = 0u;
            attack_target::SyncActivePool(current, maximum, curHp, bigHp);
            check(curHp == 123u && bigHp == 0u, "authoritative snapshots do not activate a legacy big pool");
        }

    unsigned int pendingHp = 100u;
    attack_target::ApplyPendingHPDelta(pendingHp, -55, 1000u);
    check(pendingHp == 45u, "pending HP subtracts projected damage");
    attack_target::ApplyPendingHPDelta(pendingHp, 200, 1000u);
    check(pendingHp == 245u, "pending HP adds signed healing");
    attack_target::ApplyPendingHPDelta(pendingHp, 1000, 200u);
    check(pendingHp == 200u, "pending HP clamps healing to max HP");
    attack_target::ApplyPendingHPDelta(pendingHp, -500, 200u);
    check(pendingHp == 0u, "pending HP clamps damage at zero");
    pendingHp = 100u;
    attack_target::SubtractPendingHP(pendingHp, 55);
    check(pendingHp == 45u, "pending HP uses the projected regular-pool damage");
    attack_target::SubtractPendingHP(pendingHp, 100);
    check(pendingHp == 0u, "pending HP does not underflow on overkill");
    return failures;
}
