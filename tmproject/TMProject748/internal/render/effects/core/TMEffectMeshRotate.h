#pragma once

#include "TMEffect.h"
#include "Structures.h"

class TMObject;
class TMEffectBillBoard;

// Efeito orbital que gira uma mesh e, opcionalmente, um billboard ao redor de
// um proprietário. O efeito é visual e não altera estado autoritativo do jogo.
class TMEffectMeshRotate : public TMEffect
{
public:
    // pOwner é uma referência observada durante a animação e pode deixar de
    // estar disponível no teardown; a classe não assume ownership do objeto.
    TMEffectMeshRotate(TMVector3 vecPosition, int nType, TMObject* pOwner, int bFire, int bCenter);
    ~TMEffectMeshRotate();
    
    int Render() override;
    int FrameMove(unsigned int dwServerTime) override;
    // Atualiza os canais ARGB empregados na renderização.
    void SetColor(unsigned int dwColor);

public:
    // m_pBillBoard é um efeito auxiliar opcional gerido junto desta instância.
    TMEffectBillBoard* m_pBillBoard;
    TMVector3 m_vecStartPos;
    unsigned int m_dwStartTime;
    unsigned int m_dwRotateTime;
    unsigned int m_dwLifeTime;
    int m_bCenter;
    int m_bScale;
    float m_fAngle;
    float m_fAngle2;
    float m_fRadius;
    int m_nType;
    int m_nMeshIndex;
    float m_fScale;
    unsigned int m_dwA;
    unsigned int m_dwR;
    unsigned int m_dwG;
    unsigned int m_dwB;
};
