#pragma once
// Widget control; split from SControl.h/.cpp. Include "SControl.h" for the whole library.
#include "SControlBase.h"

class SPanel : public SControl
{
public:
    SPanel(int inTextureSetIndex, float inX, float inY, float inWidth, float inHeight, unsigned int idwColor, RENDERCTRLTYPE eRenderType);
    ~SPanel();

    int OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY) override;
    virtual void SetTextureSetIndex(int inTextureSetIndex);
    virtual GeomControl* GetGeomControl();

    void SetVisible(int bVisible) override;
    void FrameMove2(stGeomList* pDrawList, TMVector2 ivParentPos, int inParentLayer, int nFlag) override;

public:
    GeomControl m_GCPanel;
    SPanel* m_pDescPanel;
    int m_nPickPosX;
    int m_nPickPosY;
    int m_bPickable;
    int m_bPicked;
};
