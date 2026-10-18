#pragma once

namespace scrollbar_layout {
// Native UI2 uses a 17-unit button with a 7-unit texture offset.
// Constructors use resource units (scale 1); SetSize receives physical pixels.
constexpr float DownArrowY(float height, float scaleY)
{
    return height - (17.0f - 7.0f) * scaleY;
}
}
