#pragma once

#include "TMEffect.h"
#include "Structures.h"

class TMShade;
class TMEffectBillBoard;

// Composição visual do golpe Double Swing. Posição, nível e proprietário são
// entradas para a animação; acerto e dano permanecem sob decisão do servidor.
class TMSkillDoubleSwing : public TMEffect
{
public:
    // pOwner é referência não proprietária usada para acompanhar a origem.
    TMSkillDoubleSwing(TMVector3 vecStart, TMVector3 vecTarget, int nLevel, TMObject* pOwner);
    ~TMSkillDoubleSwing();

    int Render() override;
    int IsVisible() override;
    int FrameMove(unsigned int dwServerTime) override;

public:
    // Os ponteiros de luz são efeitos auxiliares opcionais e podem ser nulos
    // durante criação parcial ou teardown.
    unsigned int m_dwStartTime;
    TMVector3 m_vecStartPos;
    TMVector3 m_vecTargetPos;
    int m_nLevel;
    int m_nMeshIndex;
    float m_fAngle;
    float m_fAngle2;
    float m_fProgress;
    unsigned int m_dwLifeTime;
    unsigned int m_dwOldTime;
    TMShade* m_pLightMap;
    TMEffectBillBoard* m_pCenterLight;
    TMEffectBillBoard* m_pCenterLight2;
    TMEffectBillBoard* m_pCenterFlare;
};
