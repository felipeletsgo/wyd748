#pragma once
// Widget control; split from SControl.h/.cpp. Include "SControl.h" for the whole library.
#include "SPanel.h"
#include "SText.h"

class SButton : public SPanel
{
public:
    enum {
        TMC_IMAGE_COMMON = 0x0,
        TMC_IMAGE_OVER = 0x1,
        TMC_IMAGE_PRESS = 0x2,
        TMC_IMAGE_SELECTED = 0x3,
    };
    enum { TMC_BUTTON_CLICK = 0x0 };

public:
    SButton(int inTextureSetIndex, float inX, float inY, float inWidth, float inHeight,
        unsigned int idwColor, int bSound, char* istrText);
    ~SButton();
    virtual void SetText(char* istrText);
    int OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY) override;
    virtual void SetSelected(int bSelected);
    void FrameMove2(stGeomList* pDrawList, TMVector2 ivParentPos, int inParentLayer, int nFlag) override;
    void Update() override;

public:
    int m_bMouseOver;
    int m_bSelected;
    char m_cAlwaysAlt;
    char m_cBlink;
    char m_cBlink1;
    char m_cBlink2;
    SText* m_pAltText;
    int m_bPressed;
    int m_GrayType;

protected:
    unsigned int m_dwColor;
    int m_bSound;
    unsigned int m_dwOldTime;
};
