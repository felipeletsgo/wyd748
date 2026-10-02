#pragma once
// Widget control; split from SControl.h/.cpp. Include "SControl.h" for the whole library.
#include "SPanel.h"
#include "SText.h"
#include "SButton.h"

class SMessageBox : public SPanel, public IEventListener
{
public:
    enum {
        TMC_OKBUTTON_ID = 0x1,
        TMC_CANCELBUTTON_ID = 0x2,
    };
    enum {
        TMC_MESSAGEBOX_MESSAGE = 0,
        TMC_MESSAGEBOX_ASK = 1,
        TMC_MESSAGEBOX_SLIDE = 2,
        TMC_MESSAGEBOX_COMPLEX = 3,
        TMC_MESSAGEBOX_OK = 4,
    };
    enum {
        TMC_MESSAGE_OK = 0x0,
        TMC_MESSAGE_CANCEL = 0x1,
    };

public:
    SMessageBox(const char* istrMessage, char ibyMessageBoxType, float inX, float inY);
    ~SMessageBox();

    int OnControlEvent(DWORD idwControlID, DWORD idwEvent) override;
    virtual void SetMessage(char* istrMessage, unsigned int dwMessageValue, char* istrMessage2);
    virtual void SetMessage(unsigned int dwMessageValue);
    virtual unsigned int GetMessageA();
    int OnCharEvent(char iCharCode, int lParam) override;
    void FrameMove2(stGeomList* pDrawList, TMVector2 ivParentPos, int inParentLayer, int nFlag) override;
    void SetVisible(int bVisible) override;
    int OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY) override;

public:
    unsigned int m_dwArg;
    unsigned int m_dwMessage;
    char m_byMessageBoxType;

protected:
    SPanel* m_pPanel1;
    SPanel* m_pPanel2;
    SText* m_pMessage;
    SText* m_pMessage2;
    SText* m_pCaption;
    SButton* m_pOKButton;
    SButton* m_pCancelButton;
    SPanel* m_pPanelBtn1;
    SPanel* m_pPanelBtn2;
};
