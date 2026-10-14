#pragma once

#include "TMEffect.h"
#include "WeatherParticles.h"

// Rain effect of the active scene. The drops, their speeds and the draw batch
// live inside the object; the count follows ClientSettings WeatherArea.
class TMRain : public TMEffect
{
public:
	TMRain();
	~TMRain();
	void SetActive(bool active);

	// Draws the current drops in one batched call; returns the TMEffect code.
	int Render() override;

	// Moves and recycles the drops with the server time.
	int FrameMove(unsigned int dwServerTime) override;

private:
	void StopAmbientSound();

public:
	unsigned int m_dwOldServerTime;   // Time of the previous update.
	int m_nDrops;                     // Active drops (native 50).
	float m_fHalfSize;                // Half size of the area around the player.
	float m_fSpeed[weather_area::kMaxRainDrops];              // Fall speed per drop.
	TMVector3 m_vecRainPosition[weather_area::kMaxRainDrops]; // Position relative to the player.
	RDLVERTEX m_vertex[4];            // Billboard quad of one drop.
	RDLVERTEX m_batch[weather_area::kMaxRainDrops * 6];       // Triangle list sent per frame.
};
