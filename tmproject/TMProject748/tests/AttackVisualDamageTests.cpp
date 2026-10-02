#include "../internal/wire/AttackVisualDamage.h"

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
    return failures;
}
