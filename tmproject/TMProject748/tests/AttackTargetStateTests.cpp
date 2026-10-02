#include "../internal/application/AttackTargetState.h"

#include <cstdio>
#include <initializer_list>

namespace
{
struct Score { unsigned int MaxHP; unsigned int CurHP; };
struct Target { Score m_stScore; unsigned int m_BigHp; unsigned int m_MaxBigHp; };
struct Dam { short Damage; };

// Literal copies of the original OnPacketAttack statements. Their implicit
// signed/unsigned conversions are the behavior under test, so only this
// oracle suppresses the corresponding warnings.
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

        pTargetHuman->m_stScore.CurHP = (short)(pTargetHuman->m_BigHp / nDamageRate);
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
            pTargetHuman->m_stScore.CurHP = (short)(pTargetHuman->m_BigHp / nDamageRate);
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
        pTargetHuman->m_stScore.CurHP = (short)pTargetHuman->m_BigHp;
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
    const int rates[]{-128, -3, -1, 1, 2, 3, 127};

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
                            check(Same(actual, legacy) && actualRate == legacyRate,
                                "healing matches legacy arithmetic");
                        }

                        Target legacy{{100u, curHp}, bigHp, bigPool};
                        Target actual = legacy;
                        const int legacyValue = LegacyDamage(&legacy, dam, rate);
                        const int actualValue = attack_target::Damage(actual.m_stScore.CurHP,
                            actual.m_BigHp, actual.m_MaxBigHp != 0, damage, rate);
                        check(Same(actual, legacy) && actualValue == legacyValue,
                            "first-loop damage matches legacy arithmetic and returned value");

                        Target legacySecond{{100u, curHp}, bigHp, bigPool};
                        Target actualSecond = legacySecond;
                        LegacySubtract(&legacySecond, dam, rate);
                        if (!actualSecond.m_MaxBigHp)
                            attack_target::SubtractPool(actualSecond.m_stScore.CurHP, damage, rate);
                        else
                            attack_target::SubtractBigPool(actualSecond.m_stScore.CurHP,
                                actualSecond.m_BigHp, damage);
                        check(Same(actualSecond, legacySecond), "second-loop subtraction matches legacy arithmetic");
                    }

    // A zero rate is normalized only on the big-pool healing path and persists.
    unsigned int curHp = 10u, bigHp = 50u;
    int rate = 0;
    attack_target::HealBigPool(curHp, bigHp, 10, rate);
    check(rate == 1 && bigHp == 40u && curHp == 40u, "big-pool healing normalizes a zero rate in place");
    // Overflowing big pools pass through short: 40000 wraps to a large unsigned HP.
    curHp = 0u; bigHp = 40001u;
    attack_target::SubtractBigPool(curHp, bigHp, 1);
    check(curHp == static_cast<unsigned int>(static_cast<short>(40000)), "big pool truncates through short");
    // The unsigned "<= 0" test only clears an exact zero difference.
    curHp = 5u;
    attack_target::SubtractPool(curHp, 10, 1);
    check(curHp == 0xFFFFFFFBu, "over-subtraction wraps like the original unsigned field");
    return failures;
}
