#include "../internal/wire/AttackVisualDamage.h"
#include "../internal/application/AttackVisualTiming.h"
#include "../internal/application/AttackSpeedPolicy.h"

#include <array>
#include <cstdio>
#include <cstring>

namespace
{
// Independent oracle for the original scene helper's byte-reading algorithm.
// Literal offsets, capacities and signature deliberately do not use the policy.
int LegacyDamage(const unsigned char* bytes, unsigned int size,
    unsigned int opcode, int index, int legacy)
{
    if (!bytes || index < 0 || index >= 13)
        return 0;
    if (legacy < 0)
        return legacy;
    unsigned int damage = 0;
    if (opcode == 0x39D && size == 52) {
        std::memcpy(&damage, bytes + 48, sizeof(damage));
    } else {
        unsigned int base = 0;
        if (opcode == 0x39D) base = 48;
        else if (opcode == 0x39E) base = 52;
        else if (opcode == 0x36C) base = 96;
        if (!base || size < base + 8)
            return legacy;
        unsigned int signature = 0;
        unsigned int count = 0;
        std::memcpy(&signature, bytes + base, sizeof(signature));
        std::memcpy(&count, bytes + base + 4, sizeof(count));
        if (signature != 0x58474D44 || count > 13 ||
            static_cast<unsigned int>(index) >= count || size < base + 8 + count * 4)
            return legacy;
        std::memcpy(&damage, bytes + base + 8 + index * 4, sizeof(damage));
    }
    return damage > 0x7FFFFFFF ? legacy : static_cast<int>(damage);
}
}

int RunAttackVisualDamageTests(int& checks)
{
    static_assert(sizeof(unsigned int) == 4 && sizeof(int) == 4,
        "The oracle represents the existing Win32 rendering path");
    int failures = 0;
    const auto check = [&](bool passed, const char* message) {
        ++checks;
        if (!passed) {
            ++failures;
            std::fprintf(stderr, "FAIL attack visual damage: %s\n", message);
        }
    };
    const unsigned int values[]{0, 32767, 32768, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF};
    for (unsigned int opcode : {0x39Du, 0x39Eu, 0x36Cu, 0x123u})
    for (unsigned int count : {0u, 1u, 2u, 7u, 13u, 14u, 0xFFFFFFFFu})
    for (unsigned int signature : {0x58474D44u, 0x58474D45u})
    for (unsigned int value = 0; value < 6; ++value) {
        std::array<unsigned char, 256> bytes{};
        const unsigned int base = opcode == 0x39D ? 48 : opcode == 0x39E ? 52 : 96;
        std::memcpy(bytes.data() + base, &signature, sizeof(signature));
        std::memcpy(bytes.data() + base + 4, &count, sizeof(count));
        for (unsigned int target = 0; target < 13; ++target)
            std::memcpy(bytes.data() + base + 8 + target * 4,
                &values[(value + target) % 6], sizeof(unsigned int));
        const auto before = bytes;
        for (unsigned int size : {0u, 47u, 48u, 51u, 52u, 55u, 56u, 59u,
            60u, 63u, 64u, 67u, 68u, 95u, 96u, 103u, 104u, 107u, 108u,
            155u, 156u, 159u, 160u})
        for (int index = -1; index <= 13; ++index)
        for (int legacy : {-1000, -1, 0, 32767})
            check(attack_visual::ReadDamage(bytes.data(), size, opcode, index, legacy) ==
                LegacyDamage(bytes.data(), size, opcode, index, legacy),
                "physical/DMGX prefixes, count bounds, truncation and signed fallback match the original");
        check(bytes == before, "visual projection never modifies the borrowed packet");
    }
    for (int index : {-1, 0, 12, 13})
    for (int legacy : {-1000, -1, 0, 32767})
        check(attack_visual::ReadDamage(nullptr, 156, 0x36C, index, legacy) == 0,
            "absent packet is rejected before all legacy and tail reads");
    std::array<unsigned char, 52> physical{};
    const unsigned int wide = 60000;
    std::memcpy(physical.data() + 48, &wide, sizeof(wide));
    check(attack_visual::ReadDamage(physical.data(), 52, 0x39D, 0, 32767) == 60000,
        "compact physical hit reads the DWORD without requiring a DMGX header");
    check(attack_visual::ReadDamage(physical.data(), 52, 0x39D, 12, 32767) == 60000,
        "compact physical branch retains the original index behavior");
    check(attack_visual::ReadDamage(physical.data(), 52, 0x39D, 0, -1) == -1,
        "negative legacy damage precedes a valid physical extension");
    check(attack_visual::ReadDamage(physical.data(), 0, 0x39D, 0, 32767) == 32767,
        "local prediction with zero header size keeps its legacy visual damage");

    // Heals read the real 32-bit amount from the same DMGX slot.
    std::array<unsigned char, 60> heal{};
    const unsigned int dmgx = 0x58474D44, one = 1, healAmount = 250000;
    std::memcpy(heal.data() + 48, &dmgx, sizeof(dmgx));
    std::memcpy(heal.data() + 52, &one, sizeof(one));
    std::memcpy(heal.data() + 56, &healAmount, sizeof(healAmount));
    check(attack_visual::ReadHeal(heal.data(), 60, 0x39D, 0, -32767) == -250000,
        "heal reads the real 32-bit amount beyond the saturated word");
    check(attack_visual::ReadHeal(heal.data(), 60, 0x39D, 0, -3) == -3 &&
        attack_visual::ReadHeal(heal.data(), 60, 0x39D, 0, 500) == 500,
        "miss sentinels and damage values are not heals");
    check(attack_visual::ReadHeal(heal.data(), 0, 0x39D, 0, -700) == -700 &&
        attack_visual::ReadHeal(heal.data(), 60, 0x39D, 1, -700) == -700 &&
        attack_visual::ReadHeal(nullptr, 60, 0x39D, 0, -700) == -700,
        "local previews, absent targets and packets keep the legacy heal");
    const unsigned int zero = 0, huge = 0x80000000u;
    std::memcpy(heal.data() + 56, &zero, sizeof(zero));
    check(attack_visual::ReadHeal(heal.data(), 60, 0x39D, 0, -700) == -700,
        "an old server's zero heal tail keeps the legacy heal");
    std::memcpy(heal.data() + 56, &huge, sizeof(huge));
    check(attack_visual::ReadHeal(heal.data(), 60, 0x39D, 0, -700) == -700,
        "an unrepresentable heal tail keeps the legacy heal");
    for (unsigned int firstLife : {1000u, 1200u, 1400u, 2100u, 2200u})
    for (unsigned int firstCreated : {500u, 10000u, 0xFFFFFFC0u})
    for (int secondOffset : {-20, 0, 50, 60}) {
        attack_visual::DoubleHitNumbers timing;
        unsigned int created = firstCreated;
        unsigned int lifetime = firstLife;
        int viewHalf = 0;
        int speed = static_cast<int>(firstLife / 700);
        int startY = 500;
        timing.Schedule(0, created, lifetime, viewHalf, speed, startY);
        check(created == firstCreated && lifetime == firstLife && viewHalf == 0,
            "first impact keeps its original delay, lifetime and visibility");
        created += 25; // Simulate constructing the second number in a later tick.
        lifetime += 1100;
        viewHalf = 1;
        speed = static_cast<int>(lifetime / 700);
        startY += secondOffset;
        timing.Schedule(1, created, lifetime, viewHalf, speed, startY);
        check(created - firstCreated == 120 && lifetime == firstLife && viewHalf == 0,
            "normal and critical follow-ups share the first curve, exactly 120 ms later including tick wrap");
        check(speed == static_cast<int>(firstLife / 700) && startY == 500,
            "incoming, outgoing and ground pairs share cached speed and initial height");
        for (float scale : {1.0f, 1.25f, 1.5f})
        for (int distance : {100, 120}) {
            const auto y = [&](unsigned int age) {
                return startY - speed * (static_cast<float>(age) / lifetime) * distance * scale;
            };
            for (unsigned int age = firstLife / 2; age + 50 < firstLife; age += 50) {
                const unsigned int secondAge = age - (created - firstCreated);
                check(y(secondAge) > y(age),
                    "the delayed number stays below the first throughout their shared visible trajectory");
                const float delta = (y(secondAge + 50) - y(secondAge)) - (y(age + 50) - y(age));
                check(delta > -0.001f && delta < 0.001f,
                    "both numbers rise at equal velocity for normal and critical distances and UI scales");
            }
        }
    }
    attack_visual::DoubleHitNumbers unpaired;
    unsigned int created = 1000, lifetime = 2200;
    int viewHalf = 1;
    int speed = 3, startY = 550;
    unpaired.Schedule(1, created, lifetime, viewHalf, speed, startY);
    check(created == 1000 && lifetime == 2200 && viewHalf == 1 && speed == 3 && startY == 550,
        "missing first number does not invent a follow-up timeline");
    for (unsigned int dex : {0u, 50u, 250u, 300u, 499u, 500u, 505u, 545u, 550u, 1000u}) {
        const int percent = attack_speed::Percent(dex, 0, 0, 0, 0, 0, 0, 0, false);
        const unsigned int band = (dex / 5 > 150 ? 150 : dex / 5) / 10;
        const int chance = attack_speed::DoubleChancePerMille(percent, band << 4);
        const int expected = percent > 200 ? 1000 : band < 5 ? 0 : (band - 5) * 100;
        check(percent == 100 + static_cast<int>(dex / 5) && chance == expected,
            "HUD and prediction retain exact speed and the project double-hit threshold");
        int doubles = 0;
        for (int roll = 0; roll < 1000; ++roll)
            doubles += roll < chance;
        check(doubles == expected, "every probability-table value follows the threshold");
    }
    check(attack_speed::DoubleChancePerMille(200, 0xA2) == 500 &&
        attack_speed::DoubleChancePerMille(201, 0xA2) == 1000 &&
        attack_speed::DoubleChancePerMille(209, 0xA2) == 1000,
        "201 and 209 guarantee doubles despite sharing the packed band with 200");
    check(attack_speed::Percent(505, 0, 0, 0, 0, 0, 0, 0, true) == 171 &&
        attack_speed::Percent(455, 0, 0, 0, 0, 0, 1, 0, false) == 201 &&
        attack_speed::Percent(405, 0, 0, 0, 40, 0, 0, 0, false) == 201 &&
        attack_speed::Percent(405, 0, 0x80000, 0, 26, 1, 0, 0, false) == 201 &&
        attack_speed::Percent(305, 0, 0x200000, 0, 33, 0, 0, 0, false) == 201 &&
        attack_speed::Percent(355, 0, 0, 0, 63, 0, 0, 0, false) == 201,
        "buff, freeze and transformation bonuses retain the existing HUD calculation");
    for (int weapon : {0, 1, 41, 101})
    for (int motions : {0, 1, 2, 3})
    for (bool frozen : {false, true}) {
        const unsigned int base = frozen ? 46u : 40u;
        const unsigned int expectedDouble = static_cast<unsigned int>(
            static_cast<float>(base) / (weapon == 101 ? 2.0f : 1.7f));
        check(attack_visual::AttackFrameStep(40, frozen, motions, true, weapon) == expectedDouble,
            "first and follow-up motions use double speed despite previous skill motion count");
        check(attack_visual::AttackFrameStep(40, frozen, motions, false, weapon) ==
            (motions > 1 ? static_cast<unsigned int>(static_cast<float>(base) / 1.2f) : base),
            "single attacks, ordinary skills and freeze retain existing frame timing");
    }
    return failures;
}
