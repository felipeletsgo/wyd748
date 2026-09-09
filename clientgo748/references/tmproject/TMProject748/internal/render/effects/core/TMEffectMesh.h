#pragma once

#include "TMEffect.h"

class TMMesh;

// Efeito baseado em mesh com cor, escala e rotação próprias. A malha apontada
// é obtida do sistema de recursos; a instância não é sua proprietária.
class TMEffectMesh : public TMEffect
{
public:
    TMEffectMesh(int nMeshIndex, unsigned int dwColor, float fAngle, int nType);
    ~TMEffectMesh();

    int Render() override;
    int FrameMove(unsigned int dwServerTime) override;
    // Atualiza a cor ARGB e os canais usados durante Render.
    void SetColor(unsigned int dwColor);
    int IsVisible() override;
    int IsInView();

public:
    // m_pMesh pode ser nulo se o recurso não carregar; consumidores devem usar
    // IsVisible/Render em vez de assumir que a malha está disponível.
    TMMesh* m_pMesh;
    int m_nMeshIndex;
    unsigned int m_dwColor;
    float m_fAngle;
    float m_fAngle2;
    float m_fAngle3;
    float m_fScaleH;
    float m_fScaleV;
    int m_nType;
    int m_nTextureIndex;
    float m_fProgress;
    char m_cShine;
    char m_cUScroll;
    unsigned int m_dwLifeTime;
    unsigned int m_dwCreateTime;
    unsigned int m_dwCycleTime;
    unsigned int m_dwA;
    unsigned int m_dwR;
    unsigned int m_dwG;
    unsigned int m_dwB;
};
