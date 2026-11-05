#pragma once

namespace desktop_gamma {
// Windowed brightness changes the ramp of the whole display. Capture the
// user's ramp once, before the first change, and restore exactly that ramp.
// A failed capture blocks the change, so the client never replaces a ramp
// that it cannot restore.
template<class Ramp>
class Saved {
public:
    template<class Read>
    bool Capture(Read read)
    {
        if (captured_) return true;
        Ramp pending{};
        if (!read(pending)) return false;
        ramp_ = pending;
        captured_ = true;
        return true;
    }

    // Returns true when nothing remains to restore.
    template<class Write>
    bool Restore(Write write)
    {
        if (!captured_) return true;
        if (!write(static_cast<const Ramp&>(ramp_))) return false;
        captured_ = false;
        return true;
    }

    bool Captured() const { return captured_; }

private:
    Ramp ramp_{};
    bool captured_ = false;
};
}
