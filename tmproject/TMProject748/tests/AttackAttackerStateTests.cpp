#include "../internal/application/AttackAttackerState.h"

#include <cstdio>

namespace
{
struct FakeSwing
{
    char m_cSForce = -7;
    unsigned int m_dwStartTime = 0xDEADBEEFu;
};

// Independent oracle copied from the original OnPacketAttack branches.
int LegacySwingForce(int meshType, int actorClass, int leftWeaponType)
{
    if (meshType == 3 || meshType == 8 || actorClass == 40)
        return 5;
    else if (leftWeaponType == 101)
        return 2;
    else if (leftWeaponType == 41)
        return 3;
    else if (leftWeaponType == 103)
        return 4;
    return 1;
}

bool LegacyAppliesMana(bool localAttacker, char flagLocal, short skillIndex)
{
    return (!localAttacker || !flagLocal && localAttacker) &&
        skillIndex >= 0 && skillIndex < 104;
}

bool LegacyDispatchesVisuals(bool localAttacker, char flagLocal, char motion)
{
    return !localAttacker || flagLocal == 1 && localAttacker ||
        !flagLocal && localAttacker && (unsigned char)motion == 254;
}

static_assert(attack_attacker::SwingForce(3, 0, 0) == 5, "Mesh 3 swing force");
static_assert(attack_attacker::SwingForce(8, 0, 101) == 5, "Mesh 8 precedes weapon");
static_assert(attack_attacker::SwingForce(0, 40, 103) == 5, "Class 40 precedes weapon");
static_assert(attack_attacker::SwingForce(0, 0, 101) == 2, "Weapon 101 swing force");
static_assert(attack_attacker::SwingForce(0, 0, 41) == 3, "Weapon 41 swing force");
static_assert(attack_attacker::SwingForce(0, 0, 103) == 4, "Weapon 103 swing force");
static_assert(attack_attacker::SwingForce(0, 0, 0) == 1, "Default swing force");
static_assert(!attack_attacker::ShouldApplyMana(false, 0, -1), "Negative skill rejects mana");
static_assert(attack_attacker::ShouldApplyMana(false, 1, 103), "Upper mana boundary");
static_assert(!attack_attacker::ShouldApplyMana(false, 0, 104), "Excluded mana boundary");
static_assert(!attack_attacker::ShouldApplyMana(true, 1, 0), "Local echo keeps predicted mana");
static_assert(attack_attacker::ShouldDispatchVisuals(true, 0, 254), "Local server skill visual");
static_assert(!attack_attacker::ShouldDispatchVisuals(true, 0, 253), "Local non-skill echo");
}

int RunAttackAttackerStateTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool passed, const char* message) {
        ++checks;
        if (!passed) {
            ++failures;
            std::fprintf(stderr, "FAIL attack attacker state: %s\n", message);
        }
    };

    const int meshes[]{-1, 0, 2, 3, 4, 7, 8, 9, 20, 255};
    const int classes[]{-1, 0, 32, 39, 40, 41};
    const int weapons[]{-1, 0, 40, 41, 42, 100, 101, 102, 103, 104};
    for (int mesh : meshes)
        for (int actorClass : classes)
            for (int weapon : weapons)
                check(attack_attacker::SwingForce(mesh, actorClass, weapon) ==
                    LegacySwingForce(mesh, actorClass, weapon), "swing force matches legacy precedence");

    // Both, either and neither swing effects: each present effect gets the same
    // force and start time; an absent one is not dereferenced.
    for (int mask = 0; mask < 4; ++mask) {
        FakeSwing first, second;
        attack_attacker::ApplySwing(mask & 1 ? &first : nullptr,
            mask & 2 ? &second : nullptr, 4, 1234u);
        check((first.m_cSForce == 4) == ((mask & 1) != 0), "first swing force applied only when present");
        check((first.m_dwStartTime == 1234u) == ((mask & 1) != 0), "first swing time applied only when present");
        check((second.m_cSForce == 4) == ((mask & 2) != 0), "second swing force applied only when present");
        check((second.m_dwStartTime == 1234u) == ((mask & 2) != 0), "second swing time applied only when present");
    }

    const char flags[]{-1, 0, 1, 2};
    for (int local = 0; local < 2; ++local) {
        for (char flag : flags) {
            for (int skill = -2; skill <= 160; ++skill) {
                const bool expected = LegacyAppliesMana(local != 0, flag, static_cast<short>(skill));
                check(attack_attacker::ShouldApplyMana(local != 0, flag, skill) == expected,
                    "mana gate matches legacy condition");

                const unsigned short received = 0xFFFFu;
                unsigned int actorMana = 7u;
                int localCalls = 0;
                unsigned short localValue = 0;
                const bool applied = attack_attacker::ApplyMana(local != 0, flag, skill,
                    received, actorMana, [&](unsigned short value) { ++localCalls; localValue = value; });
                check(applied == expected, "mana application result");
                check(actorMana == (expected ? 65535u : 7u), "actor mana is zero-extended or untouched");
                check(localCalls == (expected && local ? 1 : 0), "local snapshot callback only for applied local mana");
                if (localCalls)
                    check(localValue == received, "local snapshot receives the packet mana");
            }
        }
    }

    // Callback order: the actor value must already be applied when the local
    // UI/snapshot update runs, because that update reads the actor score.
    unsigned int actorMana = 0u;
    attack_attacker::ApplyMana(true, 0, 10, 321, actorMana,
        [&](unsigned short) { check(actorMana == 321u, "actor mana precedes local update"); });

    for (int local = 0; local < 2; ++local)
        for (char flag : flags)
            for (int motion = -128; motion < 128; ++motion)
                check(attack_attacker::ShouldDispatchVisuals(local != 0, flag,
                    static_cast<unsigned char>(motion)) ==
                    LegacyDispatchesVisuals(local != 0, flag, static_cast<char>(motion)),
                    "visual dispatch matches legacy condition");

    return failures;
}
