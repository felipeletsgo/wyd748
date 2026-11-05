#pragma once
#include <algorithm>

namespace effect_emission {
// Compatible cosmetic policy, not a claim about native monitor refresh.
// Keep the existing 30-Hz visual reference. Drop backlog beyond 100 ms.
class Clock {
    unsigned int previous_ = 0;
    unsigned int credit_ = 0;
    bool started_ = false;
public:
    unsigned int Steps(unsigned int now, bool active = true) {
        if (!active) { previous_ = now; credit_ = 0; started_ = false; return 0; }
        if (!started_) { previous_ = now; started_ = true; return 1; }
        const unsigned int elapsed = now - previous_;
        previous_ = now;
        // A backward server-clock correction is not a stalled frame.
        if (elapsed > 0x7fffffffu) { credit_ = 0; return 0; }
        credit_ += (std::min)(elapsed, 100u) * 30u;
        const unsigned int count = credit_ / 1000u;
        credit_ %= 1000u;
        return count;
    }
};
}
