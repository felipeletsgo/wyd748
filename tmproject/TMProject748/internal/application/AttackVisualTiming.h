#pragma once

namespace attack_visual
{
    // Presentation only: the server still resolves one authoritative action.
    constexpr unsigned int DoubleHitGapMs = 120;

    class DoubleHitNumbers
    {
    public:
        void Schedule(int hit, unsigned int& created, unsigned int& lifetime,
            int& viewHalf, int& speed, int& startY)
        {
            if (hit == 0) {
                firstCreated_ = created;
                firstLifetime_ = lifetime;
                firstSpeed_ = speed;
                firstStartY_ = startY;
                hasFirst_ = true;
            } else if (hasFirst_) {
                // Both numbers use the same visibility curve. Extending the
                // second lifetime used to delay its appearance by up to 1 s.
                created = firstCreated_ + DoubleHitGapMs;
                lifetime = firstLifetime_;
                // TMFont3 caches speed in its constructor before this call.
                // Matching lifetime alone leaves the follow-up moving faster.
                speed = firstSpeed_;
                startY = firstStartY_;
                viewHalf = 0;
            }
        }

    private:
        unsigned int firstCreated_ = 0;
        unsigned int firstLifetime_ = 0;
        int firstSpeed_ = 0;
        int firstStartY_ = 0;
        bool hasFirst_ = false;
    };

    inline unsigned int AttackFrameStep(unsigned int base, bool frozen,
        int motionCount, bool doubleHit, int weaponType)
    {
        if (frozen)
            base = static_cast<unsigned int>(static_cast<float>(base) * 1.15f);
        // A previous multi-motion skill must not override double-hit timing.
        if (doubleHit)
            base = static_cast<unsigned int>(static_cast<float>(base) /
                (weaponType == 101 ? 2.0f : 1.7f));
        else if (motionCount > 1)
            base = static_cast<unsigned int>(static_cast<float>(base) / 1.2f);
        return base;
    }
}
