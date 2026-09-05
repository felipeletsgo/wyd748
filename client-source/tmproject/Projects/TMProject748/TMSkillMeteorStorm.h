#pragma once

#include "TMEffect.h"

class TMObject;
class TMShade;
class TMEffectBillBoard;

// Efeito visual da Meteor Storm entre origem e destino. O proprietário é usado
// apenas como referência de cena; resolução da skill pertence ao servidor.
class TMSkillMeteorStorm : public TMEffect
{
public:
    // pOwner não tem ownership transferido para o efeito.
    TMSkillMeteorStorm(TMVector3 vecStart, TMVector3 vecTarget, int nLevel, TMObject* pOwner);
    ~TMSkillMeteorStorm();

    int Render() override;
    int IsVisible() override;
    int FrameMove(unsigned int dwServerTime) override;

public:
    // Luzes auxiliares podem ser nulas em falha parcial/teardown. A flag de som
    // impede repetição durante o ciclo de vida da mesma instância.
    unsigned int m_dwStartTime;
    TMVector3 m_vecStartPos;
    TMVector3 m_vecTargetPos;
    int m_nLevel;
    float m_fAngle;
    float m_fProgress;
    unsigned int m_dwLifeTime;
    TMShade* m_pLightMap;
    TMEffectBillBoard* m_pCenterLight;
    TMEffectBillBoard* m_pCenterLight2;
    TMEffectBillBoard* m_pCenterFlare;
    float m_fLength;
    float m_fDestLength;
    int m_bPlaySound;
};
