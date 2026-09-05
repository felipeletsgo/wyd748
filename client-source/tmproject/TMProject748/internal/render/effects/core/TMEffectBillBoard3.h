#pragma once

#include "TMEffect.h"
#include "Structures.h"

// Faixa billboard entre dois pontos, usada por feixes e rastros. A instância
// possui seus vértices e não assume ownership de recursos globais de textura.
class TMEffectBillBoard3 : public TMEffect
{
public:
    // Os vetores são copiados. Escalas definem largura/altura e dwLifeTime a
    // duração em milissegundos segundo o timer do client.
	TMEffectBillBoard3(TMVector3 vecStart, TMVector3 vecEnd, int nTextureIndex, unsigned int dwColor, unsigned int dwLifeTime, float fScaleH, float fScaleV);
	~TMEffectBillBoard3();

	int Render() override;
	// Atualiza os canais ARGB usados na próxima renderização.
	virtual void SetColor(unsigned int dwColor);
	int FrameMove(unsigned int dwServerTime) override;
	
    // Reposiciona as extremidades sem recriar o efeito.
    void SetPosition(TMVector3 vecStart, TMVector3 vecEnd);
    // Ativa a janela curta de exibição pelo intervalo informado.
    void SetShort(unsigned int dwTime);

public:
    unsigned int m_dwCreateTime;
    unsigned int m_dwLifeTime;
    RDLVERTEX m_vertex1[4];
    RDLVERTEX m_vertex2[4];
    float m_fScaleH;
    float m_fScaleV;
    int m_nTextureIndex;
    unsigned int m_dwA;
    unsigned int m_dwR;
    unsigned int m_dwG;
    unsigned int m_dwB;
    TMVector3 m_vecStartPos;
    TMVector3 m_vecEndPos;
    int m_nFade;
    unsigned int m_dwShortTime;
};
