#pragma once

#include <cstddef>

// AirMove stores only the displacement pending for the current frame.  Once
// committed to the client's position it must not be applied again. The server
// remains authoritative for the destination.
template <typename Position>
inline void ConsumeAirMoveDelta(Position& position, Position& delta)
{
    position.x += delta.x;
    position.y += delta.y;
    delta.x = 0.0f;
    delta.y = 0.0f;
}

// UpdateMount replaces the rider's mount with the temporary flight mesh and
// clears its look. Restore both fields before rebuilding the original mount.
template <typename Look>
inline void RestoreAirMoveMountVisual(int& mountSkinMeshType, Look& mountLook,
    int savedMountSkinMeshType, const Look& savedMountLook)
{
    mountSkinMeshType = savedMountSkinMeshType;
    mountLook = savedMountLook;
}

// A zero waypoint terminates a route. Never inspect the element after the
// final slot, even when all slots of a route are populated.
template <typename Waypoint, std::size_t Count>
inline bool HasNextAirMoveWaypoint(const Waypoint (&route)[Count], int current)
{
    return current >= 0 && static_cast<std::size_t>(current) < Count - 1 &&
        (route[current + 1].nX != 0 || route[current + 1].nY != 0);
}
