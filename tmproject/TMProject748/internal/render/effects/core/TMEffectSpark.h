#pragma once

#include "TMEffect.h"
#include "Structures.h"

class TMShade;
class TMObject;
class TMEffectBillBoard3;

// Set of up to five sparks linking a source to a target/position. The class
// animates local effects and neither sends nor confirms combat actions.
class TMEffectSpark : public TMEffect
{
public:
    // pTarget is observed, without ownership transfer. Colors, width,
    // count and initial progress are copied into the instance.
    TMEffectSpark(TMVector3 vecStart, TMObject* pTarget, TMVector3 vecEnd, unsigned int dwColor, unsigned int dwColor2, unsigned int dwLifeTime, float fWidth, int nSparkCount, float fProgress);
    ~TMEffectSpark();
    
    int FrameMove(unsigned int dwServerTime) override;

public:
    // Array entries are optional; the valid count is limited by
    // m_nSparkCount and by the fixed capacity of five elements.
    float m_fWidth;
    float m_fRange;
    TMEffectBillBoard3* m_pSpark[5];
    TMShade* m_pShade[5];
    unsigned int m_dwColor;
    unsigned int m_dwColor2;
    unsigned int m_dwCreateTime;
    unsigned int m_dwLifeTime;
    TMVector3 m_vecStartPos;
    TMVector3 m_vecEndPos;
    int m_nSparkCount;
};
