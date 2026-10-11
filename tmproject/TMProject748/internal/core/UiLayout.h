#pragma once

// Logical legacy layout size; this is not the window resolution.
constexpr float WYD748_UI_BASE_WIDTH = 800.0f;
constexpr float WYD748_UI_BASE_HEIGHT = 600.0f;

struct ImeCandidateRect
{
    float x, y, width, height;
};

// FUN_00493e70: one line at (28, H - 20), widened below 800 pixels.
// SText applies the product UI scale; return logical construction coordinates.
inline ImeCandidateRect ImeCandidateLayout(float screenWidth, float screenHeight, float uiScaleY)
{
    return {28.0f, screenHeight / uiScaleY - 20.0f,
        screenWidth < 800.0f ? 350.0f : 300.0f, 16.0f};
}

// SButton starts its hint at width - 10 scaled pixels. The native Party
// adjustment adds 5 once; recompute from width when opening the panel again.
inline float PartyHintOffsetX(float buttonWidth, float scale)
{
    return buttonWidth - 5.0f * scale;
}
