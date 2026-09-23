#pragma once

#include "TMScene.h"
#include "Structures.h"

class TMRain;
class TMSnow;
class SPanel;

// Demo scene inherited from a later TMProject. In this 7.48 client it remains
// an inert skeleton: the interface is retained for build compatibility, but
// the native flow has not yet been identified or adapted.
class TMDemoScene : public TMScene
{
public:
	TMDemoScene();
	~TMDemoScene();

	// Scene lifecycle entry points. While the implementation remains empty,
	// they return 0 without consuming events or creating resources.
	int InitializeScene() override;
	int OnControlEvent(unsigned int idwControlID, unsigned int idwEvent) override;
	int OnCharEvent(char iCharCode, int lParam) override;
	int OnPacketEvent(unsigned int dwCode, char* buf) override;
	int OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY) override;
	int FrameMove(unsigned int dwServerTime) override;

	// Presentation-specific operations. These intentional stubs must not gain
	// behavior without an asset, camera, and teardown contract.
	void ResetDemoPlayer();
	void ReadTimeTable();
	void CamAction();
	void ReadStrings();

public:
	// State expected by the later implementation. Pointer ownership is not
	// confirmed in this port, and the stub does not initialize these fields.
	int m_bPlayingBGM;
	unsigned int m_dwStartTime;
	TMRain* m_pRain;
	TMSnow* m_pSnow;
	char m_cStartRun;
	TMHuman* m_pCheckHumanList[50];
	stDemoHuman2 m_stDemoHuman[50];
	stMobAni m_stAniList[50][16];
	char m_cPlayedFlag[50][16];
	char m_szEndingString[500][128];
	SPanel* m_pCoverPanel;
	SText* m_pTextEnd;
};
