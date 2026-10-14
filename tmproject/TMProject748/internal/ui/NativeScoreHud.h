#pragma once

#include <cstdio>

// Presentation rules of the native 7.48 score HUD (WYD.exe FUN_004431e4 and
// FUN_004776c3). They own no controls and never decide game state: counts and
// values are projected from the authoritative Carry and score snapshots.
namespace native_score_hud {

// FUN_004431e4 walks the 9x7 Carry and totals what the Q, W and E shortcuts
// can consume. Stackable potions add their EF_AMOUNT value (zero when absent);
// the basic 400..404/415 and 405..409 potions count one per cell; any other
// ability-38 == 230 item adds its amount, or one when it has none.
struct QuickPotionCounts
{
    int hp = 0;
    int mp = 0;
    int special = 0;
};

constexpr bool IsStackedHpPotion(int index)
{
    return index == 4097 || index == 3431 || index == 3322
        || (index >= 428 && index <= 431) || (index >= 680 && index <= 685);
}

constexpr bool IsStackedMpPotion(int index)
{
    return index == 3323 || (index >= 432 && index <= 435) || (index >= 686 && index <= 691);
}

inline void AddCarryCell(QuickPotionCounts& counts, int index, int amount, int ability38)
{
    if (IsStackedHpPotion(index))
        counts.hp += amount;
    if (IsStackedMpPotion(index))
        counts.mp += amount;
    if ((index >= 400 && index <= 404) || index == 415)
        ++counts.hp;
    else if (index >= 405 && index <= 409)
        ++counts.mp;
    else if (ability38 == 230)
        counts.special += amount == 0 ? 1 : amount;
}

// While the local character is in the cancel state (TMHuman::m_cCancel == 1)
// the native HUD shows "X" for both shortcut counters instead of a number.
inline void FormatShortcutCount(char* text, size_t size, int count, bool cancelled)
{
    if (cancelled)
        std::snprintf(text, size, "X");
    else
        std::snprintf(text, size, "%d", count);
}

// The modern 7.48 HUD (global UI mode 2, forced by FUN_0055bc0a) hides control
// 1032 at bind time and writes "%u / %u" (current, next threshold) into 1031.
// The source score keeps 64-bit EXP; values inside the 7.48 range print the
// same digits, and larger ones are not truncated.
inline void FormatExpRange(char* text, size_t size, long long exp, long long nextLevelExp)
{
    std::snprintf(text, size, "%lld / %lld", exp, nextLevelExp);
}

// WYD-Go keeps Critical in tenths of a percent, the same unit as the item
// EF_CRITICAL tooltip, so a "9.6%" item adds 9.6% here. The 7.48 byte << 1
// scale capped the panel at 51%; the server caps the chance at 100.0%.
inline void FormatCritical(char* text, size_t size, unsigned int critical)
{
    const unsigned int perMille = critical > 1000 ? 1000 : critical;
    std::snprintf(text, size, "%u.%u%%", perMille / 10, perMille % 10);
}

// Mastery rows use "%3d/%-3d": the cap is left-aligned after the slash.
inline void FormatMastery(char* text, size_t size, int value, int cap)
{
    std::snprintf(text, size, "%3d/%-3d", value, cap);
}

// Character name 1057 is bracketed; MobName holds at most 15 characters.
inline void FormatCharacterName(char* text, size_t size, const char* name)
{
    std::snprintf(text, size, "[%.15s]", name ? name : "");
}

// FUN_004776c3: while the Character panel is visible, the left button is held
// and the cursor stays over one of the eight score '+' buttons, the scene
// re-issues that button's control event every 100 ms after a 1500 ms hold.
// Releasing the button or leaving every '+' resets the timer.
constexpr unsigned int kRepeatDelayMs = 1500;
constexpr unsigned int kRepeatIntervalMs = 100;

struct ButtonRepeat
{
    unsigned int time = 0;
    bool repeating = false;

    // Returns true when the held control event must be dispatched now.
    bool Tick(bool held, unsigned int now)
    {
        if (!held)
        {
            time = 0;
            repeating = false;
            return false;
        }
        if (!repeating)
        {
            if (time == 0)
                time = now;
            else if (time + kRepeatDelayMs < now)
            {
                time = now;
                repeating = true;
            }
            return false;
        }
        if (time + kRepeatIntervalMs < now)
        {
            time = now;
            return true;
        }
        return false;
    }

    void Reset()
    {
        time = 0;
        repeating = false;
    }
};

} // namespace native_score_hud
