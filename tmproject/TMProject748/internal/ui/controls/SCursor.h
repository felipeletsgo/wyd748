#pragma once
// Widget control; split from SControl.h/.cpp. Include "SControl.h" for the whole library.
#include "SPanel.h"

enum class ECursorStyle
{
    TMC_CURSOR_HAND = 0,
    TMC_CURSOR_CROSS_HAIR = 1,
    TMC_CURSOR_PICKUP = 2,
};

class SGridControlItem;
class SCursor : public SPanel
{
public:
    SCursor(int inTextureSetIndex, float inX, float inY, float inWidth, float inHeight);
    ~SCursor();

    int OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY) override;
    void FrameMove2(stGeomList* pDrawList, TMVector2 ivParenPos, int inParentLayer, int nFlag) override;
    virtual void SetPosition(int iX, int iY);
    void SetVisible(int bVisible) override;
    virtual void SetStyle(ECursorStyle eStyle);
    virtual ECursorStyle GetStyle();
    GeomControl* GetGeomControl() override;
    virtual int AttachItem(SGridControlItem* pItem);
    virtual SGridControlItem* DetachItem();

public:
    ECursorStyle m_eStyle;
    SGridControlItem* m_pAttachedItem;
    GeomControl m_GeomItem;

    static int m_nCursorType;
    static HCURSOR m_hCursor1;
    static HCURSOR m_hCursor2;
};
