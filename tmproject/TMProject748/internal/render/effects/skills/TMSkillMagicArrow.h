#pragma once

#include "TMEffect.h"

class TMObject;
class TMShade;
class TMEffectMesh;
class TMSkillMagicArrow : public TMEffect
{
public:
    TMSkillMagicArrow(TMVector3 vecStart, TMVector3 vecTarget, int nType, TMObject* pOwner);
    ~TMSkillMagicArrow();

    int Render() override;
    int IsVisible() override;
    int FrameMove(unsigned int dwServerTime) override;

private:
    // Creates the impact once, at natural expiry; never during teardown.
    void SpawnImpact();

public:
    // Types 3, 4 and 5 create no light map. Every scalar and pointer member
    // has a default initializer, so no variant reads or deletes garbage.
    unsigned int m_dwStartTime = 0;
    TMVector3 m_vecStartPos;
    TMVector3 m_vecTargetPos;
    TMShade* m_pLightMap = nullptr;
    TMEffectMesh* m_pEffectMesh = nullptr;
    int m_nType = 0;
    float m_fAngle = 0.0f;
    float m_fRotAngle = 0.0f;
    float m_fProgress = 0.0f;
    unsigned int m_dwLifeTime = 0;
    unsigned int m_dwColor = 0;
    unsigned int m_dwColor2 = 0;
};