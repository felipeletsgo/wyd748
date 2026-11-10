#pragma once
#include <algorithm>
#include <cmath>

namespace drop_search_layout {
constexpr unsigned int PanelID = 0x7F410000u;
constexpr float Width = 620.0f, Height = 360.0f;
struct Rect { float x, y, width, height; };
struct Placement { Rect panel; bool fits; };

// The existing CommonBox artwork is texture set 444, a 492x420 crop.
// Its close-button artwork occupies (214,394)-(283,414).
constexpr Rect CloseButton{214.0f * Width / 492.0f, 394.0f * Height / 420.0f,
    69.0f * Width / 492.0f, 20.0f * Height / 420.0f};

inline Placement Place(float viewportWidth, float viewportHeight, float scale,
    float topHudBottom, float bottomHudTop) {
    if (!std::isfinite(scale) || scale <= 0.0f) return {{}, false};
    const float margin = 8.0f * scale;
    const float top = (std::max)(margin, topHudBottom + margin);
    const float bottom = (std::min)(viewportHeight - margin, bottomHudTop - margin);
    const float width = Width * scale, height = Height * scale;
    return {{(viewportWidth - width) * 0.5f, top + (bottom - top - height) * 0.5f,
        width, height}, width + margin * 2 <= viewportWidth && height <= bottom - top};
}
}
