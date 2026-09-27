#pragma once

namespace mini_map_layout
{
struct Layout
{
    bool visible;
    bool expanded;
    float scale;
    float x, y, size;
};

// FUN_0044ca65: hidden -> compact -> expanded -> hidden. Geometry is in
// viewport pixels, including the resource children (never scale just the root).
constexpr Layout Next(bool visible, float scale, bool ui2, float width, float height)
{
    const bool expanded = visible && scale < 1.0f;
    const float size = expanded ? 400.0f : (ui2 ? 137.0f : 160.0f);
    return { !visible || expanded, expanded, expanded ? 1.5f : 0.6f,
        expanded ? (width - size) * 0.5f : width - size - (ui2 ? 2.0f : 4.0f),
        expanded ? (height - size) * 0.5f : (ui2 ? 23.0f : 4.0f), size };
}
}
