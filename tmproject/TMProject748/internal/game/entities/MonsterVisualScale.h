#pragma once

namespace monster_visual_scale
{
    constexpr float Base = 0.89999998f;
    constexpr float MaxCon = 5000.0f;
    constexpr float Maximum = (MaxCon / 2000.0f + 1.0f) * Base;

    inline float Limit(float scale)
    {
        // Also fail closed for NaN and positive infinity.
        return scale <= Maximum ? scale : Maximum;
    }

    // Server companions are named "Name^" (the native summon label rule):
    // BM and contract summons and the mount hatchling. A BM summon copies its
    // caster's CON, so its size must not follow CON; it keeps the base size.
    inline bool IsSummonName(const char* name)
    {
        if (name == nullptr)
            return false;
        for (const char* p = name; *p; ++p)
            if (*p == '^')
                return p != name;
        return false;
    }

    inline float FromCon(float con)
    {
        // Preserve signed-short patterns in historical monster templates.
        if (con >= 32768.0f && con < 65536.0f)
            con -= 65536.0f;
        return Limit((con / 2000.0f + 1.0f) * Base);
    }
}
