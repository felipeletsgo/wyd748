#pragma once

namespace skill_cooldown
{
    // WYD-Go validates catalog seconds without the newer client's equipment reductions.
    inline int DelaySeconds(int catalog, bool compatible, int sanc, bool jewel)
    {
        if (!compatible) {
            if (sanc >= 9 && catalog >= 2) --catalog;
            if (jewel) --catalog;
        }
        return catalog < 1 ? 1 : catalog;
    }

    inline bool Active(unsigned int now, unsigned int last, int seconds)
    {
        return last != 0 && static_cast<unsigned long long>(now - last) <
            static_cast<unsigned long long>(seconds) * 1000;
    }

    inline float Progress(unsigned int now, unsigned int last, int seconds)
    {
        if (!Active(now, last, seconds)) return 1.0f;
        const unsigned int elapsed = now - last;
        // SGrid reserves zero for no overlay; show the first frame of a cast too.
        return static_cast<float>(elapsed ? elapsed : 1) / (seconds * 1000.0f);
    }
}
