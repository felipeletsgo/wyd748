#pragma once

// Tick arithmetic for client effects. Server ticks are unsigned milliseconds
// that wrap at 2^32 (about 49.7 days of uptime) and can step back after a
// time synchronization. Compare tick differences, never absolute ticks, and
// convert to float only after the subtraction.
namespace effect_timing
{
// Signed distance from start to now. Correct across one wrap; negative while
// start is in the future or after the clock stepped back.
inline int Elapsed(unsigned int now, unsigned int start)
{
    return static_cast<int>(now - start);
}

// True once now reached start.
inline bool HasStarted(unsigned int now, unsigned int start)
{
    return Elapsed(now, start) >= 0;
}

// True when a nonzero lifetime has passed. A future start is not expired.
inline bool LifetimeExceeded(unsigned int now, unsigned int start, unsigned int lifetime)
{
    const int elapsed = Elapsed(now, start);
    return lifetime != 0 && elapsed > 0 && static_cast<unsigned int>(elapsed) > lifetime;
}

// Unsigned interval gate. It stays correct when the last tick is zero and
// the current tick has the high bit set.
inline bool IntervalElapsed(unsigned int now, unsigned int last, unsigned int interval)
{
    return now - last > interval;
}

// Elapsed milliseconds as float without the rounding of absolute ticks.
inline float ElapsedFloat(unsigned int now, unsigned int start)
{
    return static_cast<float>(now - start);
}

// Phase in [0, 1) of a repeating cycle. A zero cycle gives 0.
inline float CycleProgress(unsigned int now, unsigned int start, unsigned int cycle)
{
    if (cycle == 0)
        return 0.0f;
    return static_cast<float>((now - start) % cycle) / static_cast<float>(cycle);
}

// (start - now) modulo period. Equal to the unsigned expression while start
// is not in the past, and continuous when now passes start.
inline unsigned int CountdownPhase(unsigned int start, unsigned int now, unsigned int period)
{
    const long long distance = static_cast<int>(start - now);
    // A unit period also produces zero. Keep the divisor nonzero even after
    // MSVC inlines a caller that passes a constant zero period.
    const long long length = period == 0 ? 1 : period;
    return static_cast<unsigned int>(((distance % length) + length) % length);
}

// Progress of a shortened beam from shortTime to its expiry (create +
// lifetime). An empty or reversed window counts as complete.
inline float ShortWindowProgress(unsigned int now, unsigned int shortTime,
    unsigned int createTime, unsigned int lifetime)
{
    const int window = static_cast<int>(createTime + lifetime - shortTime);
    if (window <= 0)
        return 1.0f;
    return static_cast<float>(now - shortTime) / static_cast<float>(window);
}
}

// Weapon swing emitters (TMEffectSWSwing::FrameMove). Each buff family keeps
// its own clock, so one active family never suppresses another, and each
// emission advances only its own clock.
namespace swing_emitter
{
enum Family : int
{
    Berserk = 0,     // hand effect 1
    HandFour = 1,    // hand effect 4 (skill 19)
    Assert = 2,
    MagicWeapon = 3, // native FUN_004da607: 50 ms, three pairs
    ArmorClass = 4,
    FamilyCount = 5,
};

// Intervals in milliseconds; an emission needs strictly more time than this.
constexpr unsigned int kIntervalMs[FamilyCount] = { 100, 30, 50, 50, 100 };

struct Clock
{
    unsigned int last[FamilyCount]{};

    // Returns true and advances the family clock when its interval passed.
    bool TryEmit(Family family, unsigned int now)
    {
        if (family < 0 || family >= FamilyCount)
            return false;
        if (!effect_timing::IntervalElapsed(now, last[family], kIntervalMs[family]))
            return false;
        last[family] = now;
        return true;
    }
};
}
