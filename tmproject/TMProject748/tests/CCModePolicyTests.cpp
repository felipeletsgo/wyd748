#include "../internal/application/CCModePolicy.h"
#include "../internal/application/SkillCooldownPolicy.h"
#include "../internal/application/DamageNumberPolicy.h"
#include <climits>
#include <cstdio>

int RunCCModePolicyTests(int& checks)
{
    int failures = 0;
    auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) { ++failures; std::printf("FAIL CC: %s\n", name); }
    };
    for (int value = 0; value <= 90; value += 10) {
        check(cc_mode::NormalizeThreshold(value) == value, "valid percentage is stable");
        check(cc_mode::NextThreshold(value) == (value + 10) % 100, "step includes disabled state");
    }
    check(cc_mode::NormalizeThreshold(-1) == 0, "negative threshold disabled");
    check(cc_mode::NormalizeThreshold(INT_MAX) == 90, "large threshold bounded");
    check(cc_mode::NormalizeThreshold(59) == 50, "threshold snaps to tens");
    check(!cc_mode::ShouldFeed(1, 100, 1, 0), "disabled means no HP or hunger feeding");
    check(!cc_mode::ShouldFeed(0, 100, 1, 90), "dead mount is not fed");
    check(cc_mode::ShouldFeed(49, 100, 10, 50), "low HP triggers feeding");
    check(!cc_mode::ShouldFeed(50, 100, 10, 50), "HP boundary is strict");
    check(cc_mode::ShouldFeed(100, 100, 1, 50), "hunger triggers feeding");
    check(cc_mode::ShouldFeed(100, 100, 5, 50), "last hungry value triggers feeding");
    check(!cc_mode::ShouldFeed(100, 100, 6, 50), "sated mount is not fed");
    check(!cc_mode::ShouldFeed(100, 100, 0, 50), "absent hunger is not low hunger");
    check(!cc_mode::ShouldFeed(1, 0, 10, 50), "unknown max does not trigger HP feeding");
    check(cc_mode::ShouldFeed(1, 0, 1, 50), "hunger works independently of max HP");
    check(!cc_mode::BelowThreshold(INT_MAX, INT_MAX, 90), "threshold arithmetic does not overflow");
    check(cc_mode::Fixed == 2 && cc_mode::Cyclic == 1 && cc_mode::Free == 0,
        "movement labels match existing GameAuto branches");
    check(skill_cooldown::DelaySeconds(4, true, 9, true) == 4,
        "748 cooldown keeps catalog seconds despite sanc and jewel");
    check(skill_cooldown::DelaySeconds(4, false, 9, true) == 2,
        "source HUD reductions remain outside compatibility mode");
    check(skill_cooldown::DelaySeconds(0, true, 0, false) == 1, "minimum cast interval");
    check(skill_cooldown::DelaySeconds(-1, false, 9, true) == 1, "invalid catalog delay bounded");
    check(!skill_cooldown::Active(50, 0, 4), "unused skill has no cooldown");
    check(skill_cooldown::Active(10000, 10000, 4), "repeat in cast frame blocked");
    check(skill_cooldown::Active(12000, 10000, 4), "equipment cannot release skill early");
    check(skill_cooldown::Active(13999, 10000, 4), "last cooldown millisecond blocked");
    check(!skill_cooldown::Active(14000, 10000, 4), "skill ready at catalog boundary");
    check(!skill_cooldown::Active(16000, 10000, 4), "expired skill ready");
    check(skill_cooldown::Active(499, UINT_MAX - 500, 2), "cooldown survives timer wrap");
    check(!skill_cooldown::Active(1499, UINT_MAX - 500, 2), "wrapped cooldown expires");
    check(skill_cooldown::Progress(50, 0, 4) == 1.0f, "unused icon resets");
    check(skill_cooldown::Progress(10000, 10000, 4) > 0.0f &&
        skill_cooldown::Progress(10000, 10000, 4) < 0.001f, "first cast frame shows overlay");
    check(skill_cooldown::Progress(12000, 10000, 4) == 0.5f, "half cooldown projects half overlay");
    check(skill_cooldown::Progress(14000, 10000, 4) == 1.0f, "ready icon clears overlay");
    for (unsigned int now = 10000; now <= 15000; ++now)
        check((skill_cooldown::Progress(now, 10000, 4) < 1.0f) ==
            skill_cooldown::Active(now, 10000, 4), "bar and cast gate agree throughout cooldown");
    const int expectedSets[] = {-1, 137, 138, 139, 140, 141, 141, 138, 138};
    for (int type = 0; type <= 8; ++type)
        check(damage_number::TextureSet(type) == expectedSets[type], "damage type uses native number atlas");
    check(damage_number::TextureSet(-1) == -1 && damage_number::TextureSet(9) == -1,
        "unknown damage type has no atlas");
    return failures;
}
