#pragma once

// Drain DirectInput while a child UI owns input, then wait for button release.
// Otherwise closing that UI with a held button immediately clicks the world.
class ModalInputPolicy
{
public:
    bool Discard(bool captured, bool hasSample, unsigned int buttons)
    {
        if (captured) { awaitingRelease_ = true; return true; }
        if (!awaitingRelease_) return false;
        if (hasSample && buttons == 0) awaitingRelease_ = false;
        return true;
    }
private:
    bool awaitingRelease_ = false;
};
