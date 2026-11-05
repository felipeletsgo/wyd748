#pragma once

#include "TMEffect.h"

class TMSkillTownPortal : public TMEffect
{
public:
    TMSkillTownPortal(TMVector3 vecPosition, int nType);
    ~TMSkillTownPortal();

    int Render() override;
    int IsVisible() override;
    int FrameMove(unsigned int dwServerTime) override;
    void SetColor(unsigned int dwColor);

public:
    unsigned int m_dwStartTime;
    unsigned int m_dwLastTime;
    unsigned int m_dwLifeTime;
    float m_fAngle;
    float m_fProgress;
    int m_nType;
    unsigned int m_dwColor;
    unsigned int m_dwA;
    unsigned int m_dwR;
    unsigned int m_dwG;
    unsigned int m_dwB;
    // Per-instance state for the shared mesh 703, applied in Render.
    unsigned int m_dwMeshColor = 0;
    float m_fMeshScale = 1.0f;
};