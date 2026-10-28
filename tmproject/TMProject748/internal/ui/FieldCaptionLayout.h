#pragma once

#include "../platform/windows/SingleLineTextLayout.h"
#include <algorithm>

namespace field_caption_layout {

struct Band {
    float left, right;
    bool centered;
};

// Native SText (0040268c) draws beyond decorative tabs and serialized text
// rectangles. Reserve actual text/value space, not the artwork's notch width.
inline bool IsBoundedCaption(unsigned int parent, unsigned int control)
{
    if (parent == 513 && control >= 5768 && control <= 5785)
        return true;
    return (parent == 1905 && (control == 5786 || (control >= 5788 && control <= 5791))) ||
        (parent == 1889 && control >= 6050 && control <= 6052);
}

inline bool IsValueLabel(unsigned int parent, unsigned int control)
{
    return parent == 513 && control >= 5768 && control <= 5780;
}

// A one-pixel rectangle overlap with the next row is not a text collision.
// Native captions use sixteen-pixel rows, sometimes offset by one pixel.
inline bool SharesRow(const font_layout::Rectangle& text, const font_layout::Rectangle& other)
{
    const float distance = std::abs((text.y + text.height * 0.5f) -
        (other.y + other.height * 0.5f));
    return other.width > 0 && other.height > 0 &&
        distance < (std::min)(text.height, other.height) * 0.5f;
}

inline Band DrawingBounds(unsigned int parent, unsigned int control,
    const font_layout::Rectangle& text, float controlWidth,
    float leftPadding, float rightPadding, float uiScale)
{
    // The kingdom flag ends before x=69. A two-pixel caption inset leaves
    // complete EN/PT mastery names readable before the value text at x=154.
    if (IsValueLabel(parent, control))
        leftPadding = 2.0f * uiScale;
    return {text.x + leftPadding, text.x + controlWidth - rightPadding, false};
}

} // namespace field_caption_layout
