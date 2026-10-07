#pragma once

// The scene keeps only two terrain fields: the current one and one neighbor.
// Near a field corner the player is close to a horizontal and a vertical
// border at once; attaching one neighbor replaces the other, so asking for
// both every frame reloaded the two fields alternately on every frame. The
// policy keeps the attached neighbor until the player is clearly closer to
// the other border.
namespace ground_neighbor {

// Tiles the player must be closer to the other border before the neighbor
// that serves the current border is replaced.
constexpr float kCornerMargin = 4.0f;

// Distance (tiles) to the border that asks for a neighbor, or a negative value
// when the player is not near that border.
struct Need
{
    float distance = -1.0f;
    bool attached = false; // the neighbor on that side is already loaded

    bool Near() const { return distance >= 0.0f; }
    bool Wants() const { return Near() && !attached; }
};

// Decides whether the neighbor for `want` may be attached now. `other` is the
// border on the other axis: when its neighbor is attached and the player is
// not clearly closer to `want`, the attached neighbor stays.
inline bool ShouldAttach(const Need& want, const Need& other)
{
    if (!want.Wants())
        return false;
    if (!other.Near())
        return true;
    if (other.attached)
        return want.distance + kCornerMargin < other.distance;
    // Both missing: attach only the nearer border this frame (ties go to the
    // horizontal axis, which the caller evaluates first).
    return want.distance <= other.distance;
}

} // namespace ground_neighbor
