#pragma once

#include "TMEffect.h"
#include "Structures.h"

// Malha plana subdividida usada para sombra/luz projetada no terreno. Vértices
// e índices pertencem à instância; a textura é referenciada por índice global.
class TMShade : public TMEffect
{
public:
	// nGridNum define a subdivisão; nTextureIndex seleciona o recurso e fScale
	// controla o tamanho inicial.
	TMShade(int nGridNum, int nTextureIndex, float fScale);
	~TMShade();

	// Move a projeção no plano e devolve o status da atualização.
	virtual int SetPosition(TMVector2 vecPosition);
	int Render() override;
	// Renderiza na passagem inferior, quando exigido pela ordem da cena.
	virtual void RenderUnder();
	virtual void SetColor(unsigned int dwColor);
	int FrameMove(unsigned int dwServerTime) override;
	int IsVisible() override;

public:
	// m_pVertex e m_wpIndex são buffers internos alocados para a grade e podem
	// ser nulos antes da inicialização completa ou depois do teardown.
	int m_nTextureIndex;
	RDLVERTEX* m_pVertex;
	unsigned short* m_wpIndex;
	int m_nVertexNum;
	int m_nIndexNum;
	int m_nGridNum;
	float m_fAngle;
	unsigned int m_dwCreateTime;
	unsigned int m_dwLifeTime;
	unsigned int m_dwA;
	unsigned int m_dwR;
	unsigned int m_dwG;
	unsigned int m_dwB;
	int m_bFI;
	int m_nFade;
	float m_fScale;
};
