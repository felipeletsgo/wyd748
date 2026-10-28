#pragma once

#include <cstddef>
#include <cmath>
#include "AirMoveRoutes.h"

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

// Ground interpolation remains anchored to the server-owned departure point
// during flight. Preserve the visual position reached in earlier frames before
// consuming this frame's displacement, or every frame restarts at departure.
template <typename Position>
inline void UpdateHumanFramePosition(Position& position, const Position& ground,
    Position& flightDelta, bool flying)
{
    if (flying)
        ConsumeAirMoveDelta(position, flightDelta);
    else
        position = ground;
}

// Preserve the existing waypoint tolerance and diagonal speed. Both the scene
// and the route regression exercise this calculation without a render device.
template <typename Position, typename Waypoint>
inline Position AirMoveWaypointDelta(const Position& position,
    const Waypoint& waypoint, float speed)
{
    Position delta{};
    delta.x = position.x >= waypoint.nX ? -speed : speed;
    delta.y = position.y >= waypoint.nY ? -speed : speed;
    if (std::fabs(waypoint.nX - (position.x + delta.x)) <= 0.5f)
        delta.x = 0.0f;
    if (std::fabs(waypoint.nY - (position.y + delta.y)) <= 0.5f)
        delta.y = 0.0f;
    if (std::fabs(delta.x) == speed && std::fabs(delta.y) == speed)
    {
        delta.x /= 2.0f;
        delta.y /= 2.0f;
    }
    return delta;
}

// Death cancels the server-owned route. Discard uncommitted visual movement
// and return to the position captured before flight, never to its destination.
template <typename Position>
inline void CancelAirMoveAtOrigin(Position& position, Position& delta,
    const Position& origin)
{
    position = origin;
    delta.x = 0.0f;
    delta.y = 0.0f;
}

// An authoritative teleport has already replaced the position. Only discard
// the flight's uncommitted displacement; do not move back to its old origin.
template <typename Position>
inline void DiscardAirMoveDelta(Position& delta)
{
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
