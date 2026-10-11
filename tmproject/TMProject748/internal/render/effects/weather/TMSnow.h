#pragma once

#include "TMEffect.h"
#include "Structures.h"
#include "WeatherParticles.h"

// Snowfall around the camera or the player. The flakes, their speeds and the
// draw batch live inside the object; the count follows ClientSettings
// WeatherArea.
class TMSnow : public TMEffect
{
public:
    // fScale sets the visual size of the flakes.
    TMSnow(float fScale);
    ~TMSnow();

    // Draws the visible flakes in one batched call and drives the ambient sound.
    int Render() override;

    // Moves and recycles the flakes with the time and the current camera.
    int FrameMove(unsigned int dwServerTime) override;

public:
    unsigned int m_dwOldServerTime;    // Last processed time.
    int m_nFlakes;                     // Active flakes (native 200).
    float m_fHalfSize;                 // Half size of the area around the player.
    float m_fSpeed[weather_area::kMaxSnowFlakes];              // Fall speed per flake.
    TMVector3 m_vecSnowPosition[weather_area::kMaxSnowFlakes]; // World position.
    RDLVERTEX m_vertex[4];             // Billboard quad of one flake.
    RDLVERTEX m_batch[weather_area::kMaxSnowFlakes * 6];       // Triangle list sent per frame.
    float m_fScale;                    // Effective flake scale.
};
