#pragma once

// Compatible CC extension: only schedules existing 7.48 item/attack intentions.
namespace cc_mode
{
    constexpr int TextureSlot = 511;
    constexpr int FirstTextureSet = 550;
    constexpr int Fixed = 2;
    constexpr int Cyclic = 1;
    constexpr int Free = 0;

    constexpr bool OwnsCombat(int mode)
    {
        return mode == 1 || mode == 2;
    }

    constexpr int NormalizeThreshold(int value)
    {
        return value < 0 ? 0 : value > 90 ? 90 : value - value % 10;
    }

    constexpr int NextThreshold(int value)
    {
        return (NormalizeThreshold(value) + 10) % 100;
    }

    constexpr bool BelowThreshold(int current, int maximum, int percent)
    {
        return percent > 0 && current > 0 && maximum > 0 &&
            static_cast<long long>(current) * 100 < static_cast<long long>(maximum) * percent;
    }

    constexpr bool Hungry(int hunger)
    {
        return hunger > 0 && hunger < 6;
    }

    // TMProject reference: scan the inactive belt left to right before attacks.
    constexpr bool IsAutomaticBuff(int skill)
    {
        switch (skill) {
        // 13 Growth applies the Possessed affect (14), mapped back to skill 13.
        case 3: case 5: case 53: case 54: case 9: case 11: case 13: case 37:
        case 41: case 43: case 44: case 45: case 46: case 64: case 66:
        case 68: case 70: case 71: case 87: case 75: case 76: case 77:
        case 81: case 86: case 89: case 92: // 86 Coin Armor; 85 Backpack opens Cargo
            return true;
        default:
            return false;
        }
    }

    constexpr int BuffAffectSkill(int skill)
    {
        return skill == 64 || skill == 66 || skill == 68 || skill == 70 || skill == 71
            ? 71 : skill;
    }

    // Automatic casting repeats every frame. Without a mana gate it rebuilt a
    // chat line and replayed the error sound per frame (and per candidate mob
    // for area skills), which collapsed the frame rate.
    constexpr unsigned int ManaWarningIntervalMs = 3000;

    // Manual casts keep the native per-attempt warning; CC reports at most
    // once per interval. Unsigned subtraction handles server tick wraparound.
    constexpr bool ShouldReportInsufficientMana(bool automatic, bool reported,
        unsigned int now, unsigned int last)
    {
        return !automatic || !reported || now - last >= ManaWarningIntervalMs;
    }

    constexpr bool HasMana(long long cost, long long current)
    {
        return cost <= current;
    }

    // Affect Time is in native 8-second units, counted from the moment the
    // 0x3B9 snapshot that carried it arrived. The client never decrements the
    // stored value, so the remaining time is derived from that arrival.
    constexpr unsigned int AffectUnitMs = 8000;

    // Rebuff while at most this many units remain (24 seconds), so the new
    // cast lands before the old buff expires and the character is never
    // left unbuffed.
    constexpr int RebuffLeadUnits = 3;

    constexpr int RemainingAffectUnits(int units, unsigned int now, unsigned int received)
    {
        const long long elapsed = static_cast<long long>((now - received) / AffectUnitMs);
        return units > elapsed ? static_cast<int>(units - elapsed) : 0;
    }

    constexpr bool BuffNeedsRefresh(int remainingUnits)
    {
        return remainingUnits <= RebuffLeadUnits;
    }

    constexpr bool BuffDelayDue(unsigned int now, unsigned int lastCast, unsigned int delay)
    {
        return lastCast == 0 || now - lastCast >= delay;
    }
}
