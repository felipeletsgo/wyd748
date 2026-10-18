#pragma once

class ScopedReentry
{
public:
    explicit ScopedReentry(bool& active) : active_(active), entered_(!active)
    {
        if (entered_) active_ = true;
    }
    ~ScopedReentry() { if (entered_) active_ = false; }
    explicit operator bool() const { return entered_; }
    ScopedReentry(const ScopedReentry&) = delete;
    ScopedReentry& operator=(const ScopedReentry&) = delete;
private:
    bool& active_;
    bool entered_;
};
