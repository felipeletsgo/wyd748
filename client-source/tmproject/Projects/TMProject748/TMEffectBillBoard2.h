#pragma once

#include "TMEffect.h"
#include "Structures.h"

// Billboard plano com escala, rotação e fade animados. Usa textura indexada
// pelo gerenciador global e mantém localmente apenas vértices e parâmetros.
class TMEffectBillBoard2 : public TMEffect
{
public:
    // As escalas e velocidades são copiadas; nenhum recurso externo é adotado.
    TMEffectBillBoard2(int nTextureIndex, unsigned int dwLifeTime, float fScaleX, float fScaleY, float fScaleZ, float fVelocity, unsigned int dwVel);
    ~TMEffectBillBoard2();

    int Render() override;
    // Decompõe ARGB nos canais mantidos pela instância.
    virtual void SetColor(unsigned int dwColor);
    int FrameMove(unsigned int dwServerTime) override;
    // Substitui a duração usada por FrameMove sem recriar o billboard.
    virtual void SetLifeTime(unsigned int dwLifeTime);

public:
    RDLVERTEX m_vertex[4];
    TMVector3 m_vecScale;
    float m_fScaleVel;
    unsigned int m_dwLifeTime;
    unsigned int m_dwCreateTime;
    unsigned int m_dwRotVel;
    int m_nTextureIndex;
    float m_fAxisAngle;
    int m_nFade;
    int m_bSlope;
    float m_fLocalHeight;
    float m_fProgress;
    unsigned int m_dwA;
    unsigned int m_dwR;
    unsigned int m_dwG;
    unsigned int m_dwB;
};
