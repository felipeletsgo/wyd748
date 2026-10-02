#pragma once
// Widget control; split from SControl.h/.cpp. Include "SControl.h" for the whole library.
#include "SControlBase.h"

class SText : public SControl
{
public:
    enum {
        TEXT_TYPE_NORMAL = 0x0,
        TEXT_TYPE_SHADOW = 0x1,
        TEXT_TYPE_TRANS = 0x2,
        TEXT_TYPE_FOCUS = 0x3
    };
    enum {
        TEXT_ALIGN_LEFT = 0x0,
        TEXT_ALIGN_CENTER = 0x1,
        TEXT_ALIGN_RIGHT = 0x2,
        TEXT_ALIGN_NOMARGINE = 0x3,
        TEXT_ALIGN_BATTLE = 0x4,
    };

public:
    SText(int inTextureSetIndex, const char* istrText, unsigned int idwFontColor, float inX, float inY,
        float inWidth, float inHeight, int ibBorder, unsigned int idwBorderColor, unsigned int dwType, unsigned int dwAlignType);
    ~SText();
    virtual void SetText(char* istrText, int bCheckZero);
    virtual void SetTextColor(unsigned int dwFontColor);
    virtual char* GetText();
    virtual void SetType(unsigned int dwType);
    void FrameMove2(stGeomList* pDrawList, TMVector2 ivParentPos, int inParentLayer, int nFlag) override;
    int OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY) override;

public:
    TMFont2 m_Font;
    TMFont2 m_Font2;
    TMFont2 m_Font3;
    TMFont2 m_Font4;
    GeomControl m_GCText;
    GeomControl m_GCText2;
    GeomControl m_GCText3;
    GeomControl m_GCText4;
    unsigned int m_dwAlignType;
    unsigned int m_dwTextType;
    char m_cBorder;
    char m_cComma;
    GeomControl m_GCBorder;
};
