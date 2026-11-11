#pragma once

// Movement decisions of TMHuman without scene or Direct3D dependencies.
namespace transition_gate
{
    // Transition stamps are server times; zero means "not set". Compare the
    // elapsed time so a server-tick wrap cannot keep the gate closed. The
    // drift correction can move the clock back by a few hundred ms; a stamp
    // less than `delay` ahead of `now` is therefore still a fresh stamp.
    inline bool HasElapsed(unsigned int now, unsigned int stamp, unsigned int delay)
    {
        if (stamp == 0)
            return true;
        const unsigned int elapsed = now - stamp;
        if (elapsed > 0u - delay)
            return false;
        return elapsed >= delay;
    }
}

namespace ground_route
{
    constexpr bool MayReplan(bool moving, float progress, bool newClick,
        bool lastStep, bool segmentReachesDestination)
    {
        if (!moving)
            return true;
        if (segmentReachesDestination || (!newClick && !lastStep))
            return false;
        return progress == 0.0f || progress >= 0.9f;
    }
}

namespace route_bridge
{
    constexpr int kRouteCells = 48;

    // Joins a bridge route table (from the actor's cell to route[joinIndex])
    // with the remaining server route. Max indices use GenerateRouteTable's
    // convention: step count + 2. Returns the joined max index, or 0.
    template <class Vec>
    int Compose(const Vec* bridge, int bridgeMaxIndex, const Vec* route, int joinIndex,
        int routeMaxIndex, Vec* out)
    {
        const int bridgeSteps = bridgeMaxIndex - 2;
        if (!bridge || !route || !out || bridgeSteps < 0 || bridgeSteps >= kRouteCells ||
            joinIndex < 0 || joinIndex >= kRouteCells)
            return 0;

        for (int i = 0; i < bridgeSteps; ++i)
            out[i] = bridge[i];

        Vec current = bridge[bridgeSteps];
        for (int i = bridgeSteps; i < kRouteCells; ++i)
        {
            const int source = i - bridgeSteps + joinIndex;
            if (source < kRouteCells)
                current = route[source];
            out[i] = current;
        }

        int maxIndex = routeMaxIndex - joinIndex + bridgeSteps;
        if (maxIndex < bridgeMaxIndex)
            maxIndex = bridgeMaxIndex;
        if (maxIndex > kRouteCells - 1)
            maxIndex = kRouteCells - 1;
        return maxIndex;
    }
}
