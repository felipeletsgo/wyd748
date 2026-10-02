#pragma once
// Widget control; split from SControl.h/.cpp. Include "SControl.h" for the whole library.
#include "SControlBase.h"
#include "SButton.h"

class SButtonBox : public SControl
{
public:
    SButtonBox(float inPosX, float inPosY, float inWidth, float inHeight, int nStartCount, int nEndCount, int nCurrnetPage, int nPrevPage,
        int nNextPage, int nStartPage, int nEndPage);
    ~SButtonBox();

    void SetEventListener(IEventListener* ipEventListener) override;
    void SetButtonBox(int nStartCount, int nEndCount, int nCurrnetPage, int nPrevPage, int nNextPage, int nStartPage, int nEndPage);

public:
    SButton* m_pButtons[14];
    int m_nCurrentPage;
    int m_nStartCount;
    int m_nEndCount;
    int m_nStartPage;
    int m_nEndPage;
    int m_nPrevPage;
    int m_nNextPage;
};
