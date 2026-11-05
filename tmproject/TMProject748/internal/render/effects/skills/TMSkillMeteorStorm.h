#pragma once
#include "../../../application/EffectEmissionClock.h"

#include "TMEffect.h"

class TMObject;
class TMShade;
class TMEffectBillBoard;

// Efeito visual da Meteor Storm entre origem e destino. O proprietário é usado
// apenas como referência de cena; resolução da skill pertence ao servidor.
class TMSkillMeteorStorm : public TMEffect
{
    effect_emission::Clock m_emissionClock;
public:
    // pOwner não tem ownership transferido para o efeito.
    TMSkillMeteorStorm(TMVector3 vecStart, TMVector3 vecTarget, int nLevel, TMObject* pOwner);
    ~TMSkillMeteorStorm();

    int Render() override;
    int IsVisible() override;
    int FrameMove(unsigned int dwServerTime) override;

private:
    // Creates the impact once, at natural expiry; never during teardown.
    void SpawnImpact();

public:
    // Luzes auxiliares podem ser nulas em falha parcial/teardown. A flag de som
    // impede repetição durante o ciclo de vida da mesma instância.
    unsigned int m_dwStartTime = 0;
    TMVector3 m_vecStartPos;
    TMVector3 m_vecTargetPos;
    int m_nLevel = 0;
    float m_fAngle = 0.0f;
    float m_fProgress = 0.0f;
    unsigned int m_dwLifeTime = 0;
    TMShade* m_pLightMap = nullptr;
    TMEffectBillBoard* m_pCenterLight = nullptr;
    TMEffectBillBoard* m_pCenterLight2 = nullptr;
    TMEffectBillBoard* m_pCenterFlare = nullptr;
    float m_fLength = 0.0f;
    float m_fDestLength = 0.0f;
    int m_bPlaySound = 0;
};
