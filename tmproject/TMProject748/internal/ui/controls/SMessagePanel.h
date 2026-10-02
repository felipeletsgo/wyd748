#pragma once
// Widget control; split from SControl.h/.cpp. Include "SControl.h" for the whole library.
#include "SPanel.h"
#include "SText.h"

class SMessagePanel : public SPanel
{
public:
    SMessagePanel(const char* istrMessage, float inX, float inY, float inWidth, float inHeight, unsigned int dwTime);
    ~SMessagePanel();

    void SetMessage(const char* istrMessage, unsigned int dwTime);
    void FrameMove2(stGeomList* pDrawList, TMVector2 ivParentPos, int inParentLayer, int nFlag) override;
    virtual void SetVisible(int bVisible, int bSound);

public:
    SText* m_pText;
    SText* m_pText2;
    SPanel* m_pPanelL;
    SPanel* m_pPanelR;
    unsigned int m_dwOldServerTime;
    unsigned int m_dwLifeTime;
};
