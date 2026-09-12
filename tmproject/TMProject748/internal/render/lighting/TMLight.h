#pragma once

#include "TreeNode.h"
#include "Structures.h"

// Luz pontual administrada pelo renderer da cena.
//
// A classe sincroniza uma D3DLIGHT9 com o dispositivo, aplica a cor inicial ao
// terreno e evita ativar luzes fora da regiao visivel da camera. Cena, camera e
// dispositivo sao servicos globais consultados, nao recursos possuidos aqui.
class TMLight : public TreeNode
{
public:
	TMLight(D3DCOLORVALUE col, int bEnable);
	~TMLight();

	virtual void InitPosition(float fX, float fY, float fZ);
	int FrameMove(unsigned int dwServerTime) override;
	virtual int IsVisible();

public:
	float m_fRadius;
	int m_bEnable;
	int m_bVisible;
	unsigned int m_dwLightIndex;
	D3DLIGHT9 m_Light;
	TMVector2 m_vecPosition;

	// Proximo slot Direct3D reservado externamente por TMObjectContainer.
	static unsigned int m_dwBaseLightIndex;
};
