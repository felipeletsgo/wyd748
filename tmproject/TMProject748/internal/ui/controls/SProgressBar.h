#pragma once
// Widget control; split from SControl.h/.cpp. Include "SControl.h" for the whole library.
#include "SPanel.h"

class SProgressBar : public SPanel
{
public:
    enum {
        PROGRESSBAR_DEFAULT_COLOR = 0x11111111,
    };
    enum {
        TMPROGRESS_STYLE_V = 0x0,
        TMPROGRESS_STYLE_H = 0x1,
        TMPROGRESS_STYLE_VH = 0x2,
    };
public:
    SProgressBar(int inTextureSetIndex, int inCurrent, int inMax, float inX, float inY, float inWidth, float inHeight,
        unsigned int idwProgressColor, unsigned int idwColor, unsigned int dwStyle);
    ~SProgressBar();

    virtual void SetCurrentProgress(int inCurrent);
    virtual void SetMaxProgress(int inMax);
    virtual int GetCurrentProgress();
    virtual int GetMaxProgress();
    void Update() override;
    void FrameMove2(stGeomList* pDrawList, TMVector2 ivParentPos, int inParentLayer, int nFlag) override;
    virtual void ResetBar();

public:
    unsigned int m_dwStyle;
    int m_nCurrent;
    int m_nMax;
    float m_nProgressWidth;
    float m_nProgressHeight;
    GeomControl m_GCProgress;
    float m_InitHeight;
    float m_InitStartY;
    float m_InitWidth;
    float m_InitStartX;
};
