#pragma once

#include <cmath>
#include <string>

namespace font_layout {

struct Rectangle {
    float x, y, width, height;
};

inline float WithinPanel(const Rectangle& text, float panelWidth, float margin)
{
    if (text.x < 0.0f || panelWidth <= 0.0f)
        return text.width;
    const float remaining = panelWidth - margin - text.x;
    return remaining < 0.0f ? 0.0f : (remaining < text.width ? remaining : text.width);
}

// Legacy label rectangles can overlap the next value or button. Reserve its
// horizontal space only when both controls occupy the same row.
inline float BeforeNeighbor(const Rectangle& text, float width, const Rectangle& neighbor)
{
    if (neighbor.width <= 0.0f || neighbor.height <= 0.0f || neighbor.x <= text.x ||
        neighbor.y >= text.y + text.height || text.y >= neighbor.y + neighbor.height)
        return width;
    const float remaining = neighbor.x - text.x;
    return remaining < width ? remaining : width;
}

// Use rasterized glyph widths, including every horizontal texture chunk.
inline float Measure(const short* chunks, int count)
{
    float width = 0.0f;
    if (!chunks)
        return width;
    for (int i = 0; i < count && i < 3; ++i)
        if (chunks[i] > 0)
            width += chunks[i];
    return width;
}

// CP1252 captions contain one byte per character. Preserve complete glyphs at
// the caller's original scale instead of shrinking their rasterized texture.
// The source remains owned by the caller so wider bounds restore it in full.
template<class MeasureText>
std::string Ellipsize(const std::string& source, float scale, float maximumWidth,
    MeasureText measure)
{
    if (!std::isfinite(maximumWidth) || maximumWidth <= 0.0f || source.empty())
        return source;
    if (!std::isfinite(scale) || scale <= 0.0f)
        return {};
    const auto fits = [&](const std::string& text) {
        const float width = measure(text);
        return std::isfinite(width) && width >= 0.0f && width * scale + 1.0f <= maximumWidth;
    };
    if (fits(source))
        return source;
    if (!fits("..."))
        return {};
    for (auto length = source.size(); length > 0; --length) {
        auto display = source.substr(0, length - 1) + "...";
        if (fits(display))
            return display;
    }
    return "...";
}

} // namespace font_layout
