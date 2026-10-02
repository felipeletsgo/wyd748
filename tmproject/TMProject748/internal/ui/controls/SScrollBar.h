#pragma once
// Widget control; split from SControl.h/.cpp. Include "SControl.h" for the whole library.
#include "SControlBase.h"
#include "SPanel.h"

class SScrollBar : public SControl, public IEventListener
{
public:
    enum {
        SCROLLBAR_DEFAULT_COLOR = 0x11111111,
    };
    enum {
        TMSCROLL_STYLE_V = 0x0,
        TMSCROLL_STYLE_H = 0x1,
    };
    enum {
        TMC_PANEL_UP = 0x0,
        TMC_PANEL_DOWN = 0x1,
        TMC_PANEL_BAR = 0x2,
        TMC_PANEL_BACK1 = 0x3,
        TMC_PANEL_BACK2 = 0x4,
    };
public:
    SScrollBar(int inCurrent, int inMax, float inX, float inY, float inWidth, float inHeight,
        unsigned int dwStyle, unsigned int idwBarColor, unsigned int idwColor, int bChat);
    ~SScrollBar();

    virtual void SetCurrentPos(int inCurrent);
    virtual int GetCurrentPos();
    virtual void SetMaxValue(int inMax);
    virtual int GetMaxValue();
    void SetSize(float nWidth, float nHeight) override;
    virtual void Up();
    virtual void Down();
    void Update() override;
    int OnControlEvent(DWORD idwControlID, DWORD idwEvent) override;
    void FrameMove2(stGeomList* pDrawList, TMVector2 ivParentPos, int inParentLayer, int nFlag) override;
    int OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY) override;

    void upbarSetPos(float x, float y);
    void downbarSetPos(float x, float y);
    void upbarSetsize(float x, float y);
    void downbarSetsize(float x, float y);


    void upbarSetvisible(bool bSet);
    void downbarSetvisible(bool bSet);
    void scrollbarSetvisible(bool bSet);
    void scrollbarbackSetvisible(bool bSet);

protected:
    unsigned int m_dwStyle;
    int m_nCurrent;
    int m_nMax;
    int m_nScrollPos;
    int m_nScrollLength;
    float m_nBarSize;
    float m_nBtnSize;
    SPanel* m_pUpPanel;
    SPanel* m_pDownPanel;
    SPanel* m_pBar;

public:
    SPanel* m_pBackground1;
};
