#include "../internal/application/CCModePolicy.h"
#include "../internal/application/SkillCooldownPolicy.h"
#include "../internal/application/DamageNumberPolicy.h"
#include <climits>
#include <cstdio>
#include <initializer_list>

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
    check(!cc_mode::BelowThreshold(1, 100, 0), "disabled percentage suppresses HP feeding");
    check(!cc_mode::BelowThreshold(0, 100, 90), "dead mount has no HP feeding");
    check(cc_mode::BelowThreshold(49, 100, 50), "low HP triggers feeding");
    check(!cc_mode::BelowThreshold(50, 100, 50), "HP boundary is strict");
    check(cc_mode::Hungry(1) && cc_mode::Hungry(5), "reference hunger range is independent of percentage");
    check(!cc_mode::Hungry(0) && !cc_mode::Hungry(6), "absent and sated hunger do not trigger feeding");
    check(!cc_mode::BelowThreshold(1, 0, 50), "unknown max does not trigger HP feeding");
    check(!cc_mode::BelowThreshold(INT_MAX, INT_MAX, 90), "threshold arithmetic does not overflow");
    check(cc_mode::RemainingAffectUnits(10, 16000, 0) == 8, "remaining time counts 8-second units");
    check(cc_mode::RemainingAffectUnits(2, 80000, 0) == 0, "an elapsed buff has no time left");
    check(cc_mode::RemainingAffectUnits(5, 7999u, 0xFFFFFFFFu) == 4, "arrival time survives tick wraparound");
    check(!cc_mode::BuffNeedsRefresh(4), "a buff with more than 24 seconds left is kept");
    check(cc_mode::BuffNeedsRefresh(3), "a buff is refreshed before it expires");
    check(cc_mode::BuffNeedsRefresh(0), "a missing buff is cast");
    const int referenceBuffs[] = {3, 5, 53, 54, 9, 11, 37, 41, 43, 44, 45, 46,
        64, 66, 68, 70, 71, 87, 75, 76, 77, 81, 85, 89, 92};
    for (int skill = -1; skill < 256; ++skill) {
        bool expected = false;
        for (int reference : referenceBuffs) expected |= skill == reference;
        check(cc_mode::IsAutomaticBuff(skill) == expected, "automatic buff set matches pinned reference");
    }
    for (int skill : {64, 66, 68, 70, 71})
        check(cc_mode::BuffAffectSkill(skill) == 71, "all transformations share one active affect");
    check(cc_mode::BuffAffectSkill(44) == 44, "ordinary buff keeps its own affect identity");
    for (int skill : {3, 5, 53, 54, 74, 76, 77})
        check(cc_mode::BuffMasterySlot(skill) == 1, "first mastery group matches reference");
    for (int skill : {9, 11, 13, 15, 37, 86, 87})
        check(cc_mode::BuffMasterySlot(skill) == 2, "second mastery group matches reference");
    for (int skill : {41, 43, 44, 45, 46, 64, 66, 68, 70, 71, 89, 90})
        check(cc_mode::BuffMasterySlot(skill) == 3, "third mastery group matches reference");
    for (int skill : {75, 81, 85, 92})
        check(cc_mode::BuffDelayMs(skill, 150, 320) == 0, "reference has no mastery delay for this buff");
    check(cc_mode::BuffDelayMs(3, 2, 100) == 20000, "reference delay is 100 times AffectTime times mastery");
    check(cc_mode::BuffDelayMs(3, 150, 320) == 576000, "reference delay cannot outlast server refresh window");
    check(cc_mode::BuffDelayMs(3, INT_MAX, INT_MAX) == 576000, "delay multiplication cannot overflow");
    check(cc_mode::BuffDelayMs(3, -1, 100) == 0 && cc_mode::BuffDelayMs(3, 150, -1) == 0,
        "invalid catalog or mastery cannot create a huge unsigned delay");
    check(cc_mode::BuffDelayDue(1, 0, 576000), "unused buff does not wait for server uptime");
    check(!cc_mode::BuffDelayDue(29999, 10000, 20000), "mastery delay blocks an early repeat");
    check(cc_mode::BuffDelayDue(30000, 10000, 20000), "mastery delay opens at its exact boundary");
    check(cc_mode::BuffDelayDue(499, UINT_MAX - 500, 1000), "mastery delay survives timer wraparound");
    for (int mastery = 0; mastery <= 320; ++mastery)
        check(cc_mode::BuffDelayMs(44, 150, mastery) <= (38u + 37u * mastery / 320u - 3u) * 8000u,
            "every server mastery preserves the pre-expiry refresh window");
    check(cc_mode::ShouldReportInsufficientMana(false, true, 10, 10),
        "manual casts keep the native per-attempt warning");
    check(cc_mode::ShouldReportInsufficientMana(true, false, 10, 0),
        "first automatic warning is shown");
    check(!cc_mode::ShouldReportInsufficientMana(true, true, 2999, 0),
        "automatic warnings are throttled inside the interval");
    check(cc_mode::ShouldReportInsufficientMana(true, true, 3000, 0),
        "automatic warning repeats at the interval boundary");
    check(cc_mode::ShouldReportInsufficientMana(true, true, 1000, 0xFFFFF000u),
        "warning interval survives server tick wraparound");
    check(!cc_mode::ShouldReportInsufficientMana(true, true, 500, 0xFFFFFF00u),
        "wrapped interval still throttles");
    check(cc_mode::HasMana(10, 10) && !cc_mode::HasMana(11, 10) && cc_mode::HasMana(0, 0),
        "mana gate matches the cast path comparison");
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
