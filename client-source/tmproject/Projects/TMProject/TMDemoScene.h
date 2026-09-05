#pragma once

#include "TMScene.h"
#include "Structures.h"

class TMRain;
class TMSnow;
class SPanel;

// Cena de demonstração herdada do TMProject posterior. No client 7.48 deste
// projeto ela permanece como um esqueleto inerte: a interface é conservada
// para compatibilidade de compilação, mas o fluxo nativo ainda não foi
// identificado nem adaptado.
class TMDemoScene : public TMScene
{
public:
	TMDemoScene();
	~TMDemoScene();

	// Pontos de entrada do ciclo de cena. Enquanto a implementação continuar
	// vazia, retornam 0 e não consomem eventos nem criam recursos.
	int InitializeScene() override;
	int OnControlEvent(unsigned int idwControlID, unsigned int idwEvent) override;
	int OnCharEvent(char iCharCode, int lParam) override;
	int OnPacketEvent(unsigned int dwCode, char* buf) override;
	int OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY) override;
	int FrameMove(unsigned int dwServerTime) override;

	// Operações específicas da apresentação. São stubs deliberados; não devem
	// ganhar comportamento sem contrato de assets, câmera e teardown.
	void ResetDemoPlayer();
	void ReadTimeTable();
	void CamAction();
	void ReadStrings();

public:
	// Estado previsto pela implementação posterior. Os ponteiros não possuem
	// ownership confirmado no port atual e não são inicializados pelo stub.
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
