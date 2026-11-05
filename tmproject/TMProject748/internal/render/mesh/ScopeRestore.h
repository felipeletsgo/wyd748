#pragma once

#include <utility>

namespace scope_restore
{
// Runs restore once when the scope ends, unless the normal path already
// restored the state and called Dismiss.
template<class Restore>
class Guard
{
public:
    explicit Guard(Restore restore) : restore_(std::move(restore)) {}
    Guard(const Guard&) = delete;
    Guard& operator=(const Guard&) = delete;
    ~Guard()
    {
        if (armed_) restore_();
    }
    void Dismiss() { armed_ = false; }

private:
    Restore restore_;
    bool armed_ = true;
};
}
