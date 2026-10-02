#pragma once
// Widget control; split from SControl.h/.cpp. Include "SControl.h" for the whole library.
#include "SText.h"

class SEditableText : public SText
{
public:
    enum {
        TMEDIT_MSG_RETURN_PRESS = 0x0,
        TMEDIT_MSG_TAB_PRESS = 0x1,
        TMEDIT_MSG_PAGEUP_PRESS = 0x2,
        TMEDIT_MSG_UP_PRESS = 0x3,
        TMEDIT_MSG_PAGEDOWN_PRESS = 0x4,
        TMEDIT_MSG_DOWN_PRESS = 0x5,
        TMEDIT_MSG_DELETE_PRESS = 0x6,
        TMEDIT_MSG_NO_STRING = 0x7,
        TMEDIT_MSG_MAX_STRING = 0x8,
    };

public:
    SEditableText(int inTextureSetIndex, const char* istrText, size_t inMaxStringLen, int ibPasswd,
        unsigned int idwFontColor, float inX, float inY, float inWidth, float inHeight, int ibBorder,
        unsigned int idwBorderColor, unsigned int dwType, unsigned int dwAlignType);
    ~SEditableText();

    virtual void SetText(char* istrText);
    char* GetText() override;
    int OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY) override;
    int OnCharEvent(char iCharCode, int lParam) override;
    int OnChangeIME() override;
    int OnIMEEvent(char* ipComposeString) override;
    int OnKeyDownEvent(unsigned int iKeyCode) override;
    int OnKeyUpEvent(unsigned int iKeyCode) override;
    void Update() override;
    void FrameMove2(stGeomList* pDrawList, TMVector2 ivParentPos, int inParentLayer, int nFlag) override;
    void SetFocused(int bFocused) override;
    int IsIMENative() override;

public:
    char m_strText[256];
    char m_strComposeText[256];
    char m_cTempChar;
    char m_cReserved;
    size_t m_nMaxStringLen;
    int m_nCursorVisible;
    int m_bPasswd;
    int m_bEncrypt;
    int m_bKorean;
};
