#pragma once

// One outstanding authentication request per selected connection. The scene
// closes the socket on cancellation; a response without a pending request
// cannot change the selected channel or enter character selection.
class AccountLoginAttempt
{
public:
    bool Begin()
    {
        if (pending_)
            return false;
        pending_ = true;
        return true;
    }

    bool Resolve()
    {
        const bool wasPending = pending_;
        pending_ = false;
        return wasPending;
    }

    bool Pending() const { return pending_; }

private:
    bool pending_ = false;
};
