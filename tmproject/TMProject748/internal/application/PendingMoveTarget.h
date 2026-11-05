#pragma once
struct PendingMoveTarget {
    int x = 0, y = 0, count = 0, stop = 0;
    bool pending = false;
    void Set(int targetX, int targetY, int routeCount, int stopMode) {
        x = targetX; y = targetY; count = routeCount; stop = stopMode; pending = true;
    }
    bool Due(unsigned int now, unsigned int last) const {
        return pending && now - last > 1000;
    }
    void Clear() { pending = false; }
};
